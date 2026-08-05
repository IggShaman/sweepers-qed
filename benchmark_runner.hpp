#pragma once

#include "benchmark_options.hpp"
#include "field_config.hpp"

#include <vector>

namespace b
{
class benchmark_runner
{
public:
    std::expected<void, std::string> init(const CliOptions&);
    std::expected<void, std::string> run();
    std::expected<void, std::string>
    run(std::string layout_name, std::string solver_name, size_t field_config_idx, int repeat_idx);

private:
    std::vector<field_config> field_configs_;
    CliOptions options_;
    std::string make_dsn();

    int run_id_seq_{0};
};
} // namespace b
