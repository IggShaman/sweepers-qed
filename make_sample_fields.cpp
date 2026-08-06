#include "CLI/CLI.hpp"
#include "byte_field.hpp"
#include "field.hpp"
#include "field_config.hpp"
#include "field_config_io.hpp"
#include "field_stats.hpp"
#include "glpk_solver.hpp"
#include "logger.hpp"
#include "minefield_generator.hpp"
#include "scoped_timer.hpp"

#include <expected>

namespace qed
{
std::generator<qed::field_position>
find_empty_initial_pois(qed::byte_field* field, std::optional<size_t> seed)
{
    std::uniform_int_distribution<qed::index_type> y_distribution(0, field->rows() - 1);
    std::uniform_int_distribution<qed::index_type> x_distribution(0, field->columns() - 1);
    std::mt19937_64 rng{seed ? *seed : std::random_device{}()};

    while (true)
    {
        auto position = qed::field_position(y_distribution(rng), x_distribution(rng));
        auto cell = field->cell_at(position);
        if (cell.is_landmine_groundtruth() or field->nearby_landmines_count(position) != 0)
        {
            continue;
        }
        co_yield position;
    }
}

struct CliOptions
{
    int rows{256};
    int columns{256};
    double mine_ratio{0.2};
    std::uint64_t start_seed{0};
    int fields_count = 100;
    std::string output_config_file_name{"bench_field_configs.toml"};
};

std::expected<CliOptions, int> get_cli_options(int argc, char** argv)
{
    CLI::App app{"sweepers-qed sample fields config generator"};
    CliOptions opts;
    argv = app.ensure_utf8(argv);

    app.add_option("--rows", opts.rows, "board rows")->capture_default_str();
    app.add_option("--columns", opts.columns, "board columns")->capture_default_str();
    app.add_option("--mine-ratio", opts.mine_ratio, "mine ratio")
      ->check(CLI::Range(0.0, 0.3))
      ->capture_default_str();
    app.add_option("--start-seed", opts.start_seed, "initial rng seed")->capture_default_str();
    app.add_option("--fields-count", opts.fields_count, "number of fields")->capture_default_str();
    app.add_option("--output-config", opts.output_config_file_name, "save configs to toml file")
      ->capture_default_str();
    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& ex)
    {
        return std::unexpected{app.exit(ex)};
    }

    return opts;
}

std::expected<void, std::string> make_solvable_field_config(field_config& field_config)
{
    tlog << "making config for field " << field_config.name << "\n";

    auto field = std::make_shared<qed::byte_field>();
    field->reset(field_config.rows, field_config.columns);

    if (auto ok = qed::generate_minefield(
          field.get(),
          field_config.calculate_landmine_count(),
          field_config.seed);
        !ok)
    {
        return std::unexpected{I_TO_STRING("field generation failed: " << ok.error())};
    }

    for (auto initial_poi : find_empty_initial_pois(field.get(), field_config.seed))
    {
        qed::reset(field.get());

        {
            auto solver = std::make_shared<qed::GlpkSolver>();
            solver->set_byte_field(field);
            solver->startAsync();

            field->cell_at(initial_poi).set_uncovered();
            solver->addPoi(initial_poi);

            {
                qed::scoped_timer timer;
                solver->resume();
                solver->wait_for_completion();
                tlog << "runtime:" << timer.tdiff() << "\n";
            }
            solver->stop();
            solver.reset();
        }

        auto field_stats = count_field_stats(field.get());
        auto solved_ratio = static_cast<double>(field_stats.marked_as_mine) / field_stats.landmines;
        if (solved_ratio >= 0.95)
        {
            field_config.initial_pois.push_back(initial_poi);
            field_config.final_uncovered_positions = field_stats.uncovered;
            field_config.final_landmines_marked = field_stats.marked_as_mine;

            tlog << "solved_ratio=" << solved_ratio << " => accept\n" << SHOW(field_stats) << "\n";

            return {};
        }

        tlog << "solved_ratio=" << solved_ratio << ", try again\n";
    }

    return {};
}

} // namespace qed

int main(int argc, char** argv)
{
    auto config = qed::get_cli_options(argc, argv);
    if (!config)
    {
        return -1;
    }

    std::vector<qed::field_config> field_configs;
    size_t seed = config->start_seed;
    for (int i = 0; i < config->fields_count; ++i, ++seed)
    {
        std::ostringstream oss;
        oss << config->rows << 'x' << config->columns << " seed:" << seed;
        if (seed != i)
        {
            oss << " idx:" << i;
        }
        auto name = oss.str();

        qed::field_config field_config{
          .name = name,
          .rows = config->rows,
          .columns = config->columns,
          .landmine_fill_rate = config->mine_ratio,
          .seed = seed};

        if (!qed::make_solvable_field_config(field_config))
        {
            return -1;
        }

        field_configs.push_back(field_config);
    }

    if (auto ok = save_field_configs(config->output_config_file_name, field_configs); !ok)
    {
        errlog << "Could not save configs: " << ok.error() << "\n";
        return -1;
    }
}
