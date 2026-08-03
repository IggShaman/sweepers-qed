#include "benchmark_options.hpp"

#include "CLI/CLI.hpp"

namespace b
{
std::expected<CliOptions, int> get_cli_options(int argc, char** argv)
{
    CLI::App app{"sweepers-qed solver benchmark"};
    CliOptions opts;
    argv = app.ensure_utf8(argv);

    app.add_option("--rows", opts.rows, "board rows")->capture_default_str();
    app.add_option("--columns", opts.columns, "board columns")->capture_default_str();
    app.add_option("--mine-ratio", opts.mine_ratio, "mine ratio")
      ->check(CLI::Range(0.0, 0.3))
      ->capture_default_str();
    app.add_option("--seed", opts.seed, "rng seed (0 = random)")->capture_default_str();
    app.add_option("--repeats", opts.repeats, "repetitions")
      ->check(CLI::PositiveNumber)
      ->capture_default_str();
    app.add_option("--png", opts.png, "write board image here");
    app.add_flag("--verbose", opts.verbose, "verbose output");
    app.add_option("--layout", opts.layout, "storage layout")
      ->check(CLI::IsMember({"byte", "nibble", "twobit"}))
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
