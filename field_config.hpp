#pragma once

#include "field_position.hpp"

#include <vector>

namespace qed
{
struct field_config
{
    std::string name;
    qed::index_type rows;
    qed::index_type columns;
    double landmine_fill_rate;
    size_t seed;
    std::vector<qed::field_position> initial_pois;
    size_t final_uncovered_positions;
    size_t final_landmines_marked;

    size_t calculate_landmine_count() const
    {
        return static_cast<size_t>(static_cast<size_t>(rows) * columns * landmine_fill_rate);
    }
};
} // namespace qed
