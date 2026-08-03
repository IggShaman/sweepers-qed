#include "byte_field.hpp"

namespace qed
{
struct field_stats
{
    size_t landmines{};
    size_t uncovered{};
    size_t marked_as_mine{};
};

field_stats count_field_stats(auto* field)
{
    field_stats stats;

    for (qed::index_type j = 0; j < field->rows(); ++j)
    {
        for (qed::index_type i = 0; i < field->columns(); ++i)
        {
            auto cell = field->cell_at({j, i});

            if (cell.is_landmine_groundtruth())
            {
                ++stats.landmines;
            }

            if (cell.is_marked_as_landmine())
            {
                ++stats.marked_as_mine;
            }

            if (cell.is_uncovered())
            {
                ++stats.uncovered;
            }
        }
    }

    return stats;
}

inline std::ostream& operator<<(std::ostream& os, const field_stats& stats)
{
    os << "     landmines: " << stats.landmines << "\n     uncovered: " << stats.uncovered
       << "\nmarked_as_mine: " << stats.marked_as_mine;
    return os;
}
} // namespace qed
