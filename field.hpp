#pragma once

#include "field_position.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace qed
{

//
// Represents a true landmine field.
//
class Field {
public:
    void gen_random(size_t rows, size_t cols, size_t landmines_count);
    void reset(size_t rows, size_t cols);

    // for manual field control; maintains landmines_count
    void mark_landmine(FieldPosition, bool);

    bool is_landmine(FieldPosition l) const { return data_[l.row * cols_ + l.col]; }
    uint8_t nearby_landmines_count(FieldPosition) const;
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    size_t landmines_count() const { return landmines_count_; }

private:
    size_t to_index(FieldPosition) const;

    size_t landmines_count_{};
    size_t rows_{};
    size_t cols_{};
    std::vector<bool> data_;
};

using FieldPtr = std::shared_ptr<Field>;
using FieldCPtr = std::shared_ptr<const Field>;

} // namespace qed
