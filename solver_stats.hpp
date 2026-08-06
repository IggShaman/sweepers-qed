#pragma once

#include "field_config.hpp"

#include <chrono>

namespace qed
{
struct solver_run_stats
{
    int run_id;
    std::chrono::milliseconds runtime_ms;
    field_config field_config_;
    std::string solver_name;
    std::string layout_name;
};

struct solver_step_stats
{
    int run_id;
    std::chrono::milliseconds at;

    size_t frontier_size{};
    size_t uncovered_count{};
    size_t marked_count{};
};
} // namespace qed
