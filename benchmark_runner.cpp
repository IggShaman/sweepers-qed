#include "benchmark_runner.hpp"
#include "benchmark_options.hpp"
#include "benchmark_stats.hpp"
#include "byte_field_image_saver.hpp"
#include "field_config.hpp"
#include "field_config_io.hpp"
#include "field_stats.hpp"
#include "glpk_solver.hpp"
#include "logger.hpp"
#include "minefield_generator.hpp"
#include "scoped_timer.hpp"
#include "stats_sampler.hpp"

namespace qed
{
std::expected<void, std::string> benchmark_runner::init(const CliOptions& opts)
{
    options_ = opts;

    auto field_configs = load_field_configs(opts.field_config_file);
    if (!field_configs)
    {
        return std::unexpected{i::to_string(
          "Failed to load field configs from \"",
          opts.field_config_file,
          "\": ",
          field_configs.error())};
    }

    field_configs_ = *field_configs;

    return {};
}

std::expected<void, std::string> benchmark_runner::run()
{
    for (const auto& solver_name : options_.solvers)
    {
        for (const auto& layout_name : options_.layouts)
        {
            for (int field_config_idx = 0; field_config_idx < field_configs_.size();
                 ++field_config_idx)
            {
                for (int repeat_idx = 0; repeat_idx < options_.repeats; ++repeat_idx)
                {
                    auto ok = run(layout_name, solver_name, field_config_idx, repeat_idx);
                    if (!ok)
                    {
                        return std::unexpected{I_TO_STRING(
                          "Run for " << SHOW_(solver_name) << SHOW_(layout_name)
                                     << SHOW_(field_config_idx) << SHOW(repeat_idx)
                                     << " failed: " << ok.error())};
                    }
                }
            }
        }
    }

    return {};
}

std::expected<void, std::string> benchmark_runner::run(
  std::string layout_name,
  std::string solver_name,
  size_t field_config_idx,
  int repeat_idx)
{
    const auto run_id = run_id_seq_++;
    const auto dsn = I_TO_STRING(options_.experiment_path << "/stats.sqlite");

    if (layout_name != "byte")
    {
        return std::unexpected{I_TO_STRING(SHOW_(layout_name) << "is not currently supported")};
    }

    if (solver_name != "glpk")
    {
        return std::unexpected{I_TO_STRING(SHOW_(solver_name) << "is not currently supported")};
    }

    if (field_config_idx >= field_configs_.size())
    {
        return std::unexpected{
          I_TO_STRING(SHOW(field_config_idx) << " >= " << SHOW(field_configs_.size()))};
    }

    const auto& field_config = field_configs_[field_config_idx];

    tlog << "running experiment: field:" << field_config.name << " solver:" << solver_name
         << " layout:" << layout_name << " repeat:" << repeat_idx << "\n";

    auto field = std::make_shared<qed::byte_field>();
    field->reset(field_config.rows, field_config.columns);

    if (auto ok = qed::generate_minefield(
          field.get(),
          field_config.calculate_landmine_count(),
          field_config.seed);
        !ok)
    {
        return std::unexpected{I_TO_STRING("field generaton failed: " << ok.error())};
    }

    auto field_stats = count_field_stats(field.get());
    if (field_stats.landmines != field_config.calculate_landmine_count())
    {
        return std::unexpected{I_TO_STRING(
          "wrong landmine count: " << SHOW(field_stats.landmines)
                                   << " != " << SHOW(field_config.calculate_landmine_count()))};
    }

    // launch solver
    auto solver = std::make_shared<qed::GlpkSolver>();
    solver->set_byte_field(field);
    solver->startAsync();

    for (auto poi : field_config.initial_pois)
    {
        auto cell = field->cell_at(poi);
        if (cell.is_landmine_groundtruth())
        {
            errlog << "initial poi " << poi << " has a landmine\n";
            abort();
        }
        cell.set_uncovered();

        solver->addPoi(poi);
    }

    qed::stats_sampler<solver_step_stats> stats_sampler_(
      run_id,
      solver.get(),
      std::chrono::milliseconds{10});
    stats_sampler_.start();

    {
        qed::scoped_timer timer;
        solver->resume();
        solver->wait_for_completion();

        solver_run_stats run_stats{
          .run_id = run_id,
          .runtime_ms = timer.get_elapsed_ms(),
          .field_config_ = field_config,
          .solver_name = solver_name,
          .layout_name = layout_name,
        };
        tlog << "runtime:" << run_stats.runtime_ms << "\n";

        {
            qed::scoped_timer save_timer;
            if (auto ok = log_solver_run_stats(dsn, {run_stats}); !ok)
            {
                return std::unexpected{ok.error()};
            }

            auto [step_stats, _] = stats_sampler_.stop_and_take();
            if (auto ok = log_solver_step_stats(dsn, std::move(step_stats)); !ok)
            {
                return std::unexpected{ok.error()};
            }
            tlog << "stats saved in " << save_timer.get_elapsed_ms() << "\n";
        }
    }

    solver->stop();
    solver.reset();

    field_stats = count_field_stats(field.get());
    auto expected_landmine_count = field_config.calculate_landmine_count();
    if (field_stats.landmines != expected_landmine_count)
    {
        return std::unexpected{
          I_TO_STRING(SHOW(field_stats.landmines) << " != " << SHOW(expected_landmine_count))};
    }

    if (field_stats.uncovered != field_config.final_uncovered_positions)
    {
        return std::unexpected{I_TO_STRING(
          SHOW(field_stats.uncovered) << " != " << SHOW(field_config.final_uncovered_positions))};
    }

    if (field_stats.marked_as_mine != field_config.final_landmines_marked)
    {
        return std::unexpected{I_TO_STRING(
          SHOW(field_stats.marked_as_mine) << " != " << SHOW(field_config.final_landmines_marked))};
    }

    if (repeat_idx == 0 and options_.save_pngs)
    {
        auto png_file_name =
          I_TO_STRING(options_.experiment_path << '/' << field_config.name << ".png");
        if (auto ok = qed::export_field_to_png(field.get(), png_file_name); !ok)
        {
            return std::unexpected{ok.error()};
        }
    }

    return {};
}
} // namespace qed
