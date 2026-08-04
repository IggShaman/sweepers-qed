#pragma once

#include <string>

namespace qed
{
template <typename Field> void reset(Field* field)
{
    for (auto j = 0; j < field->rows(); ++j)
    {
        for (auto i = 0; i < field->columns(); ++i)
        {
            field->cell_at({j, i}).reset();
        }
    }
}
} // namespace qed
