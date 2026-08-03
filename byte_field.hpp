#pragma once

#include "aligned_bytes.hpp"
#include "field_position.hpp"
#include "neighborhood.hpp"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>

namespace qed
{
struct byte_field_cell
{
    static constexpr uint8_t kGTMask = 1 << 0;
    static constexpr uint8_t kUncoveredMask = 1 << 1;
    static constexpr uint8_t kMarkedAsLandmineMask = 1 << 2;
    static constexpr uint8_t kNearbyLandminesCountMask = 0xf0;

    byte_field_cell(std::atomic_ref<std::uint8_t> _ref) : ref{_ref} {}

    bool is_landmine_groundtruth() const;
    void set_landmine_groundtruth(bool);
    bool is_uncovered() const;
    void set_covered() const;
    void set_uncovered() const;
    bool is_marked_as_landmine() const;
    void set_marked_as_landmine() const;
    void clear_marked_as_landmine() const;
    int nearby_landmines_count_cached() const;

    std::atomic_ref<std::uint8_t> ref;
};

//
// Represents game field with one byte per cell storage format.
// Bitwise:
// 0: has-landmine, the ground truth
// 1: uncovered
// 2: marked-as-mine
// Note: combination (uncovered=1 and marked-as-mine==1) is not legit and can be
// used to encode one more extra bit of info.
// 3: unused
// bits 4..7: number of nearby landmines + 1;
//   This is computed only on demand, by calling "nearby_landmines_count(..).
//   Default-constructed backing store is all-0, so we need a flag to distinguish
//   whether "a cached value is set and is 0", versus "it has not been computed yet".
//   To do so, we store count+1 in this upper nibble, so it's >0 iff a value was
//   computed and saved. Another option would be to use bit 3 as a flag.
//
class byte_field
{
    friend struct byte_field_access;

public:
    using cell_type = byte_field_cell;

    static constexpr std::size_t alignment = 128;

    void reset(index_type rows, index_type columns);
    cell_type cell_at(field_position);
    cell_type operator[](field_position);
    int nearby_landmines_count(field_position);
    index_type rows() const { return rows_; }
    index_type columns() const { return columns_; }
    std::size_t index_at(field_position);

    std::size_t total_landmines_count{};

private:
    std::uint8_t* data() noexcept;

    index_type rows_{};
    index_type columns_{};
    index_type row_stride_{};
    i::aligned_bytes<alignment> data_;
};

inline std::ostream& operator<<(std::ostream& os, const byte_field_cell& cell)
{
    os << "[";
    if (cell.is_landmine_groundtruth())
    {
        os << "mine ";
    }

    if (cell.is_uncovered())
    {
        os << "uncovered nearby_mines:" << cell.nearby_landmines_count_cached();
    }

    if (cell.is_marked_as_landmine())
    {
        os << "marked ";
    }
    os << ']';

    return os;
}

inline std::uint8_t* byte_field::data() noexcept
{
    auto* ptr = data_.data();

    // debug-only check
    assert(i::is_aligned<alignment>(ptr));

    // release-mode promise
    return std::assume_aligned<alignment>(ptr);
}

inline bool byte_field_cell::is_landmine_groundtruth() const
{
    return ref.load(std::memory_order_relaxed) & kGTMask;
}

inline void byte_field_cell::set_landmine_groundtruth(bool value)
{
    auto v = ref.load(std::memory_order_relaxed);
    if (value)
    {
        v |= kGTMask;
    }
    else
    {
        v &= ~kGTMask;
    }
    ref.store(v, std::memory_order_relaxed);
}

inline bool byte_field_cell::is_uncovered() const
{
    return static_cast<bool>(ref.load(std::memory_order_relaxed) & kUncoveredMask);
}

inline void byte_field_cell::set_covered() const
{
    return ref.store(
      ref.load(std::memory_order_relaxed) & ~kUncoveredMask,
      std::memory_order_relaxed);
}

inline void byte_field_cell::set_uncovered() const
{
    return ref.store(
      ref.load(std::memory_order_relaxed) | kUncoveredMask,
      std::memory_order_relaxed);
}

inline bool byte_field_cell::is_marked_as_landmine() const
{
    return static_cast<bool>(ref.load(std::memory_order_relaxed) & kMarkedAsLandmineMask);
}

inline void byte_field_cell::set_marked_as_landmine() const
{
    return ref.store(
      ref.load(std::memory_order_relaxed) | kMarkedAsLandmineMask,
      std::memory_order_relaxed);
}

inline void byte_field_cell::clear_marked_as_landmine() const
{
    return ref.store(
      ref.load(std::memory_order_relaxed) & ~kMarkedAsLandmineMask,
      std::memory_order_relaxed);
}

inline int byte_field_cell::nearby_landmines_count_cached() const
{
    return static_cast<int>(ref.load(std::memory_order_relaxed) >> 4) - 1;
}

inline void byte_field::reset(index_type rows, index_type columns)
{
    rows_ = rows;
    columns_ = columns;

    row_stride_ = columns;
    auto r = columns % alignment;
    if (r > 0)
    {
        row_stride_ += alignment - r;
    }

    data_.resize(row_stride_ * rows + columns);
    size_t total_bytes = row_stride_ * rows + columns;
    auto* end_ptr = data_.data() + total_bytes;
    for (auto* ptr = data_.data(); ptr < end_ptr; ++ptr)
    {
        std::atomic_ref<uint8_t>(*ptr).store(0, std::memory_order::relaxed);
    }
}

inline std::size_t byte_field::index_at(field_position position)
{
    return position.row * row_stride_ + position.column;
}

inline byte_field::cell_type byte_field::cell_at(field_position position)
{
    return byte_field_cell{std::atomic_ref<uint8_t>(data()[index_at(position)])};
}

inline byte_field::cell_type byte_field::operator[](field_position position)
{
    return cell_at(position);
}

inline int byte_field::nearby_landmines_count(field_position position)
{
    auto cell = cell_at(position);
    const uint8_t b = cell.ref.load(std::memory_order_relaxed);
    int count = b >> 4;
    if (count > 0)
    {
        return count - 1;
    }

    for (auto neighbor_position : neighborhood(position, rows_, columns_, 1))
    {
        if (cell_at(neighbor_position).is_landmine_groundtruth())
        {
            ++count;
        }
    }

    cell.ref.store(
      (b & 0xf)            // take original flags
      | ((count + 1) << 4) // set the cached counter, +1 indicating it is set
    );

    return count;
}
} // namespace qed
