#pragma once

#include "field_position.hpp"

#include <generator>

namespace qed
{
template <typename IndexType>
std::generator<field_position_type<IndexType>> neighborhood(
  const field_position_type<IndexType> center_point,
  IndexType rows_count,
  IndexType columns_count,
  IndexType side)
{
    IndexType x_from = std::max(0, center_point.column - side);
    IndexType x_until = std::min(center_point.column + side, columns_count - 1);
    IndexType y_from = std::max(0, center_point.row - side);
    IndexType y_until = std::min(center_point.row + side, rows_count - 1);

    for (auto y = y_from; y <= y_until; ++y)
    {
        for (auto x = x_from; x <= x_until; ++x)
        {
            co_yield field_position_type<IndexType>{y, x};
        }
    }
}
} // namespace qed
