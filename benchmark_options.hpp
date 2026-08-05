#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace b
{
struct CliOptions
{
    std::string field_config_file{"bench_field_configs.toml"};
    int repeats = 5;
    std::string output_folder{"bench_outputs"};
    bool save_pngs{false};
    std::vector<std::string> layouts = {"byte"};
    std::vector<std::string> solvers = {"glpk"};
};

// This lives in a separate TU due to (alledgedly) heavy compile times with the
// CLI library.
[[nodiscard]] std::expected<CliOptions, int> get_cli_options(int argc, char** argv);
} // namespace b
