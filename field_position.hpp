#pragma once

#include <ostream>

namespace landmine {

// Represents a coordinate on a field.
struct FieldPosition {
    FieldPosition() : row{}, col{} {}
    FieldPosition(size_t _row, size_t _col) : row{_row}, col{_col} {}

    size_t row{};
    size_t col{};

    bool operator==(const FieldPosition& other) const {
        return row == other.row and col == other.col;
    }
};

inline size_t hash_mix(size_t x) { // splitmix64 finalizer
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

} // namespace landmine

namespace std {

template <> struct hash<landmine::FieldPosition> {
    std::size_t operator()(const landmine::FieldPosition& l) const {
        return landmine::hash_mix(l.row * 0x9e3779b97f4a7c15ULL + l.col);
    }
};

inline ostream& operator<<(ostream& os, const landmine::FieldPosition& l) {
    os << '(' << l.row << ' ' << l.col << ')';
    return os;
}

} // namespace std
