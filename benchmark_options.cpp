#include "benchmark_options.hpp"

#include "CLI/CLI.hpp"

namespace b
{
std::expected<CliOptions, int> get_cli_options(int argc, char** argv)
{
    CLI::App app{"sweepers-qed solver benchmark"};
    CliOptions opts;
    argv = app.ensure_utf8(argv);

    app.add_option("--field-config-file", opts.field_config_file, "toml file with field configs")
      ->capture_default_str();
    app.add_option("--output-folder", opts.output_folder, "Output folder for benchmark results")
      ->capture_default_str();
    app
      .add_option(
        "--save-pngs",
        opts.save_pngs,
        "For each board, save png file with the final result")
      ->capture_default_str();
    app.add_option("--repeats", opts.repeats, "repetitions per field")
      ->check(CLI::PositiveNumber)
      ->capture_default_str();
    app.add_option("--layout", opts.layouts, "storage layout(s)")
      ->check(CLI::IsMember({"byte", "nibble", "twobit"}))
      ->delimiter(',')
      ->capture_default_str();
    app.add_option("--solver", opts.solvers, "solver(s)")
      ->check(CLI::IsMember({"glpk", "or-tools", "highs", "scip", "soplex"}))
      ->delimiter(',')
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
