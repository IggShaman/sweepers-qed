#pragma once

#include "field_position.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

namespace qed
{

//
// Represents a true landmine field.
//
class Field {
public:
    Field() = default;
    explicit Field(uint64_t seed) : rng_{seed} {}

    void generate_random(index_type rows, index_type columns, index_type landmines_count);
    void reset(index_type rows, index_type columns);

    // for manual field control; maintains landmines_count
    void mark_landmine(FieldPosition, bool);

    bool is_landmine(FieldPosition position) const
    {
        return data_[position.row * columns_ + position.column];
    }
    uint8_t nearby_landmines_count(FieldPosition) const;
    index_type rows() const { return rows_; }
    index_type columns() const { return columns_; }
    index_type landmines_count() const { return landmines_count_; }

private:
    index_type to_index(FieldPosition) const;

    index_type landmines_count_{};
    index_type rows_{};
    index_type columns_{};
    std::vector<bool> data_;
    std::mt19937_64 rng_{std::random_device{}()};
};

using FieldPtr = std::shared_ptr<Field>;
using FieldCPtr = std::shared_ptr<const Field>;

} // namespace qed
