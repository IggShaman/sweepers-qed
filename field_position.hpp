#pragma once

#include <limits>
#include <ostream>

namespace qed
{
template <typename T> struct field_position_type
{
    static constexpr T INVALID_INDEX = std::numeric_limits<T>::max();

    field_position_type() = default;
    field_position_type(T _row, T _column) : row{_row}, column{_column} {}

    T row{INVALID_INDEX};
    T column{INVALID_INDEX};

    bool operator==(const field_position_type& other) const
    {
        return row == other.row and column == other.column;
    }

    operator bool() const { return row != INVALID_INDEX and column != INVALID_INDEX; }

    bool operator!() const { return !*this; }
};

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const field_position_type<T>& position)
{
    os << '(' << position.row << ' ' << position.column << ')';
    return os;
}

using index_type = int32_t;
using field_position = field_position_type<index_type>;

} // namespace qed

namespace std
{
template <> struct hash<qed::field_position_type<int32_t>>
{
    size_t operator()(const qed::field_position_type<int32_t>& position) const
    {
        return (static_cast<size_t>(position.row) << 32) + position.column;
    }
};
} // namespace std
