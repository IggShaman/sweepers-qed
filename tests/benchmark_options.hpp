#pragma once

#include <cstdint>
#include <expected>
#include <string>

namespace b
{
struct CliOptions
{
    int rows{512};
    int columns{512};
    double mine_ratio{0.2};
    std::uint64_t seed{0};
    int repeats = 5;
    std::string png;
    bool verbose{false};
    std::string layout = "byte";
};

// This lives in a separate TU due to (alledgedly) heavy compile times with the
// CLI library.
[[nodiscard]] std::expected<CliOptions, int> get_cli_options(int argc, char** argv);
} // namespace b
