#pragma once

#include <functional>
#include <cstddef>
#include <ostream>
#include <memory>

namespace miner
{

// Represents a coordinate on a field.
struct Location
{
    Location() : row{}, col{} {}
    Location(size_t _row, size_t _col) : row{_row}, col{_col} {}
    
    size_t row{};
    size_t col{};
    
    bool operator==(const Location& other) const
    {
        return row == other.row and col == other.col;
    }
};

inline size_t hash_mix(size_t x) {   // splitmix64 finalizer
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

} // namespace miner

namespace std {

template<>
struct hash<miner::Location> {
    std::size_t operator()(const miner::Location& l) const
    {
        return miner::hash_mix(l.row * 0x9e3779b97f4a7c15ULL + l.col);
    }
};

inline ostream& operator<<(ostream& os, const miner::Location& l) {
    os << '(' << l.row << ' ' << l.col << ')';
    return os;
}

} // namespace std

namespace miner {

//
// Represents a true mine field.
//
class Field {
public:
    void gen_random(size_t rows, size_t cols, size_t mines_nr);
    void reset(size_t rows, size_t cols);
    void mark_mined(Location, bool); // for manual minefield control; maintains mines_nr
    bool is_mined(Location l) const { return data_[l.row * cols_ + l.col]; }
    uint8_t nearby_mines_nr(Location) const;
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    size_t mines_nr() const { return mines_nr_; }
    
private:
    size_t to_index(Location) const;
    
    size_t mines_nr_{};
    size_t rows_{};
    size_t cols_{};
    std::vector<bool> data_;
};

using FieldPtr = std::shared_ptr<Field>;
using FieldCPtr = std::shared_ptr<const Field>;

} // namespace miner
