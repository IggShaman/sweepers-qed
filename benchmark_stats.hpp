#pragma once

#include "field_config.hpp"

#include <chrono>
#include <expected>
#include <string>
#include <vector>

namespace b
{
using ms_duration_type = std::chrono::duration<double, std::milli>;

struct solver_run_stats
{
    int run_id;
    ms_duration_type runtime_ms;
    field_config field_config_;
};

struct solver_step_stats
{
    int run_id;
    ms_duration_type at;

    size_t frontier_size{};
    size_t uncovered_count{};
    size_t marked_count{};
};

std::expected<void, std::string> log_build_info(const std::string& dsn);

std::expected<void, std::string>
log_solver_run_stats(const std::string& dsn, const std::vector<solver_run_stats>&);

std::expected<void, std::string>
log_solver_step_stats(const std::string& dsn, const std::vector<solver_step_stats>&);
} // namespace b
