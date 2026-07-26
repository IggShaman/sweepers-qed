#include "field.hpp"

namespace qed
{

void Field::reset(size_t rows, size_t cols)
{
    landmines_count_ = 0;
    rows_ = rows;
    cols_ = cols;
    data_.resize(rows_ * cols_);
    std::fill(data_.begin(), data_.end(), false);
}

size_t Field::to_index(FieldPosition position) const
{
    return position.row * cols_ + position.col;
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

void Field::generate_random(size_t rows, size_t cols, size_t landmines_count)
{
    srand48(time(nullptr));
    reset(rows, cols);

    // TODO: throw exception?
    if (landmines_count >= rows * cols)
    {
        return;
    }

    // For low fill rates (<= 30%), this should work well.
    landmines_count_ = landmines_count;
    while (landmines_count)
    {
        FieldPosition position{size_t(drand48() * rows_), size_t(drand48() * cols_)};
        if (is_landmine(position))
        {
            continue;
        }

        data_[to_index(position)] = true;
        --landmines_count;
    }
}

uint8_t Field::nearby_landmines_count(FieldPosition position) const
{
    uint8_t count{};

    if (position.row > 0)
    {
        if (position.col > 0)
        {
            count += is_landmine({position.row - 1, position.col - 1});
        }

        count += is_landmine({position.row - 1, position.col});

        if (position.col < cols_ - 1)
        {
            count += is_landmine({position.row - 1, position.col + 1});
        }
    }

    if (position.col > 0)
    {
        count += is_landmine({position.row, position.col - 1});
    }

    if (position.col < cols_ - 1)
    {
        count += is_landmine({position.row, position.col + 1});
    }

    if (position.row < rows_ - 1)
    {
        if (position.col > 0)
        {
            count += is_landmine({position.row + 1, position.col - 1});
        }

        count += is_landmine({position.row + 1, position.col});

        if (position.col < cols_ - 1)
        {
            count += is_landmine({position.row + 1, position.col + 1});
        }
    }

    return count;
}

} // namespace qed
