#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace qed
{
struct CliOptions
{
    std::string field_config_file{"bench_field_configs.toml"};
    int repeats = 5;
    std::string experiments_base_folder{"experiments"};
    std::string experiment_name{};
    std::string experiment_path{}; // always computed, never provided via cli
    bool save_pngs{false};
    std::vector<std::string> layouts = {"byte"};
    std::vector<std::string> solvers = {"glpk"};
};

// This lives in a separate TU due to (alledgedly) heavy compile times with the
// CLI library.
std::expected<CliOptions, std::string> get_cli_options(int argc, char** argv);
} // namespace qed
