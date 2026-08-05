#include "benchmark_runner.hpp"
#include "benchmark_options.hpp"
#include "byte_field_image_saver.hpp"
#include "field_config.hpp"
#include "field_stats.hpp"
#include "glpk_solver.hpp"
#include "logger.hpp"
#include "minefield_generator.hpp"
#include "scoped_timer.hpp"

namespace b
{
std::expected<void, std::string> benchmark_runner::init(const CliOptions& opts)
{
    options_ = opts;

    auto field_configs = b::load_field_configs(opts.field_config_file);
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
                    auto ok = run(solver_name, layout_name, field_config_idx, repeat_idx);
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
    if (layout_name != "byte")
    {
        return std::unexpected{I_TO_STRING(SHOW_(layout_name) << "is not currently supported")};
    }

    if (solver_name != "glpk")
    {
        return std::unexpected{I_TO_STRING(SHOW_(solver_name) << "is not currently supported")};
    }

    if (field_configs_.size() >= field_config_idx)
    {
        return std::unexpected{
          I_TO_STRING(SHOW(field_config_idx) << " >= " << SHOW(field_configs_.size()))};
    }

    const auto& field_config = field_configs_[field_config_idx];

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

    {
        b::scoped_timer timer;
        solver->resume();
        solver->wait_for_completion();
        tlog << "runtime:" << timer.tdiff() << "\n";
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
        if (auto ok =
              qed::export_field_to_png(field.get(), I_TO_STRING(field_config.name << ".png"));
            !ok)
        {
            return std::unexpected{ok.error()};
        }
    }

    return {};
}
} // namespace b
