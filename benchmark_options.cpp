#include "benchmark_options.hpp"
#include "logger.hpp"

#include "CLI/CLI.hpp"

#include <QDir>

#include <chrono>

namespace qed
{
std::expected<CliOptions, std::string> augment_opts(CliOptions& cli_opts)
{
    if (cli_opts.experiment_path.empty())
    {
        std::ostringstream oss;
        const auto now_ms =
          std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now());
        oss << cli_opts.experiments_base_folder << '/';
        std::print(oss, "{:%FT%TZ}", now_ms);
        cli_opts.experiment_path = oss.str();
    }

    {
        QDir dir;
        if (!dir.mkpath(QString::fromUtf8(cli_opts.experiment_path)))
        {
            return std::unexpected{I_TO_STRING("Failed to mkdir -p " << cli_opts.experiment_path)};
        }
    }

    cli_opts.experiment_name = std::filesystem::path{cli_opts.experiment_path}.filename().string();

    return {cli_opts};
}

std::expected<CliOptions, std::string> get_cli_options(int argc, char** argv)
{
    CLI::App app{"sweepers-qed solver benchmark"};
    CliOptions opts;
    argv = app.ensure_utf8(argv);

    app.add_option("--field-config-file", opts.field_config_file, "toml file with field configs")
      ->capture_default_str();
    app
      .add_option(
        "--experiments-folder",
        opts.experiments_base_folder,
        "Base folder for all experiments")
      ->capture_default_str();
    app.add_option("--experiment-name", opts.experiment_name, "Will be provided unless specified")
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
        return std::unexpected{i::to_string(ex.get_name(), ": ", ex.what())};
    }

    return augment_opts(opts);
}
} // namespace qed
