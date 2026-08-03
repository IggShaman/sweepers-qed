#pragma once

#include "field_position.hpp"

#include <cstddef>
#include <random>

namespace qed
{
// For regular fill rates (<= 20%), this should work fine.
template <typename Field>
bool generate_minefield(Field* field, size_t total_landmines_count, std::optional<size_t> seed = {})
{
    if (
      field->rows() == 0 or field->columns() == 0 or
      total_landmines_count >= field->rows() * field->columns())
    {
        return false;
    }

    field->total_landmines_count = total_landmines_count;

    std::uniform_int_distribution<index_type> y_distribution(0, field->rows() - 1);
    std::uniform_int_distribution<index_type> x_distribution(0, field->columns() - 1);
    std::mt19937_64 rng{seed ? *seed : std::random_device{}()};

    size_t placed_count = 0;
    while (placed_count < total_landmines_count)
    {
        auto position = field_position(y_distribution(rng), x_distribution(rng));
        auto cell = field->cell_at(position);
        if (cell.is_landmine_groundtruth())
        {
            continue;
        }

        cell.set_landmine_groundtruth(true);
        ++placed_count;
    }

    return true;
}
} // namespace qed
