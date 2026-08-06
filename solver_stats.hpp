#pragma once

#include "field_config.hpp"

#include <chrono>

namespace qed
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
} // namespace qed
