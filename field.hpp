#pragma once

#include "field_position.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace landmine {

//
// Represents a true mine field.
//
class Field {
public:
    void gen_random(size_t rows, size_t cols, size_t mines_nr);
    void reset(size_t rows, size_t cols);
    void mark_mined(FieldPosition, bool); // for manual minefield control; maintains mines_nr
    bool is_mined(FieldPosition l) const { return data_[l.row * cols_ + l.col]; }
    uint8_t nearby_mines_nr(FieldPosition) const;
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    size_t mines_nr() const { return mines_nr_; }
    
private:
    size_t to_index(FieldPosition) const;

    size_t mines_nr_{};
    size_t rows_{};
    size_t cols_{};
    std::vector<bool> data_;
};

using FieldPtr = std::shared_ptr<Field>;
using FieldCPtr = std::shared_ptr<const Field>;

} // namespace landmine
