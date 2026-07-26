#include "field.hpp"

namespace qed
{

void Field::reset(size_t rows, size_t cols) {
    landmines_count_ = 0;
    rows_ = rows;
    cols_ = cols;
    data_.resize(rows_ * cols_);
    std::fill(data_.begin(), data_.end(), false);
}

size_t Field::to_index(FieldPosition l) const {
    return l.row * cols_ + l.col;
}

void Field::mark_landmine(FieldPosition l, bool v) {
    auto idx = to_index(l);
    if (data_[idx]) {
        if (!v) {
            data_[idx] = false;
            landmines_count_--;
        }

    } else {
        if (v) {
            data_[idx] = true;
            ++landmines_count_;
        }
    }
}

void Field::gen_random(size_t rows, size_t cols, size_t landmines_count) {
    srand48(time(nullptr));
    reset(rows, cols);

    // TODO: throw exception?
    if (landmines_count >= rows * cols)
        return;

    // For low fill rates (<= 30%), this should work well.
    landmines_count_ = landmines_count;
    while (landmines_count) {
        FieldPosition l{size_t(drand48() * rows_), size_t(drand48() * cols_)};
        if (is_landmine(l))
            continue;

        data_[to_index(l)] = true;
        --landmines_count;
    }
}

uint8_t Field::nearby_landmines_count(FieldPosition l) const {
    uint8_t nr{};

    if (l.row > 0) {
        if (l.col > 0)
            nr += is_landmine({l.row - 1, l.col - 1});
        nr += is_landmine({l.row - 1, l.col});
        if (l.col < cols_ - 1)
            nr += is_landmine({l.row - 1, l.col + 1});
    }

    if (l.col > 0)
        nr += is_landmine({l.row, l.col - 1});

    if (l.col < cols_ - 1)
        nr += is_landmine({l.row, l.col + 1});

    if (l.row < rows_ - 1) {
        if (l.col > 0)
            nr += is_landmine({l.row + 1, l.col - 1});
        nr += is_landmine({l.row + 1, l.col});
        if (l.col < cols_ - 1)
            nr += is_landmine({l.row + 1, l.col + 1});
    }

    return nr;
}

} // namespace qed
