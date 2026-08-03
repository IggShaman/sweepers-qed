#pragma once

#include "byte_field.hpp"

namespace qed
{
struct byte_field_access
{
    static auto& get_data(byte_field& f) { return f.data_; }
    static auto get_row_stride(byte_field& f) { return f.row_stride_; }
};
} // namespace qed
