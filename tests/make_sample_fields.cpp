#include "CLI/CLI.hpp"
#include "byte_field.hpp"
#include "logger.hpp"
#include <expected>

namespace b
{
void find_empty_initial_pois(qed::byte_field* field, int find_count)
{
    int found{};

    for (auto j = static_cast<int>(field->rows() / 2) - 10;
         j < static_cast<int>(field->rows() / 2) + 10;
         ++j)
    {
        for (auto i = static_cast<int>(field->columns() / 2) - 10;
             i < static_cast<int>(field->columns() / 2) + 10;
             ++i)
        {
            auto pos = qed::field_position(j, i);
            auto cell = field->cell_at(pos);
            if (
              !cell.is_landmine_groundtruth() and field->nearby_landmines_count(pos) == 0 and
              ++found <= find_count)
            {
                errlog << "pos:" << pos << " cell:" << cell << "\n";
            }
        }
    }
}

struct CliOptions
{
    int rows{512};
    int columns{512};
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

} // namespace b

int main(int argc, char** argv)
{
    auto config = b::get_cli_options(argc, argv);
    if (!config)
    {
        return -1;
    }

    xlog << "TODO; " << config->output_config_file_name << "\n";

    // TODO: get some options, make a bunch of fields,
    // make sure they are solvable, save into a toml config
}
