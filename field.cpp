#include "field.hpp"

#include <random>

namespace qed
{
void Field::reset(index_type rows, index_type columns)
{
    landmines_count_ = 0;
    rows_ = rows;
    columns_ = columns;
    data_.resize(rows_ * columns_);
    std::fill(data_.begin(), data_.end(), false);
}

index_type Field::to_index(FieldPosition position) const
{
    return position.row * columns_ + position.column;
}

void Field::mark_landmine(FieldPosition position, bool value)
{
    auto idx = to_index(position);
    if (data_[idx] == value)
    {
        return;
    }

    data_[idx] = value;
    landmines_count_ += value ? 1 : -1;
}

void Field::generate_random(index_type rows, index_type columns, index_type landmines_count)
{
    if (rows == 0 or columns == 0 or landmines_count >= rows * columns)
    {
        return;
    }

    reset(rows, columns);

    // For low fill rates (<= 30%), this should work well.
    landmines_count_ = landmines_count;

    std::uniform_int_distribution<index_type> distribution(0, rows_ * columns_ - 1);
    while (landmines_count)
    {
        const auto idx = distribution(rng_);
        if (data_[idx])
        {
            continue;
        }

        data_[idx] = true;
        --landmines_count;
    }
}

uint8_t Field::nearby_landmines_count(FieldPosition position) const
{
    uint8_t count{};

    if (position.row > 0)
    {
        if (position.column > 0)
        {
            count += is_landmine({position.row - 1, position.column - 1});
        }

        count += is_landmine({position.row - 1, position.column});

        if (position.column + 1 < columns_)
        {
            count += is_landmine({position.row - 1, position.column + 1});
        }
    }

    if (position.column > 0)
    {
        count += is_landmine({position.row, position.column - 1});
    }

    if (position.column + 1 < columns_)
    {
        count += is_landmine({position.row, position.column + 1});
    }

    if (position.row < rows_ - 1)
    {
        if (position.column > 0)
        {
            count += is_landmine({position.row + 1, position.column - 1});
        }

        count += is_landmine({position.row + 1, position.column});

        if (position.column + 1 < columns_)
        {
            count += is_landmine({position.row + 1, position.column + 1});
        }
    }

    return count;
}

} // namespace qed
