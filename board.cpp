#include "board.hpp"

#include <iostream>

namespace qed
{

void GameBoard::uncovered_safe(qed::FieldPosition position, uint8_t nearby_landmines_count)
{
    edit_at(position) = static_cast<CellInfo>(nearby_landmines_count);
    ++uncovered_count_;
}

void GameBoard::set_field(qed::FieldPtr field)
{
    field_ = field;
    data_.resize(field_->rows() * field_->columns());
    std::fill(data_.begin(), data_.end(), CellInfo::Unknown);
}

CellNeighborhoodIterator::CellNeighborhoodIterator(GameBoard* board, qed::FieldPosition position)
    : board_{board}
{
    end_ = 0;
    if (position.row > 0)
    {
        if (position.column > 0)
        {
            neighbors_[end_++] = {
              static_cast<index_type>(position.row - 1),
              static_cast<index_type>(position.column - 1)};
        }

        neighbors_[end_++] = {static_cast<index_type>(position.row - 1), position.column};

        if (position.column + 1 < board_->columns())
        {
            neighbors_[end_++] = {static_cast<index_type>(position.row - 1), position.column + 1};
        }
    }

    if (position.column > 0)
    {
        neighbors_[end_++] = {position.row, position.column - 1};
    }

    if (position.column + 1 < board_->columns())
    {
        neighbors_[end_++] = {position.row, position.column + 1};
    }

    if (position.row + 1 < board_->rows())
    {
        if (position.column > 0)
        {
            neighbors_[end_++] = {position.row + 1, position.column - 1};
        }

        neighbors_[end_++] = {position.row + 1, position.column};

        if (position.column + 1 < board_->columns())
        {
            neighbors_[end_++] = {position.row + 1, position.column + 1};
        }
    }
}

void GameBoard::mark_mine(qed::FieldPosition position, bool value)
{
    auto& ci = edit_at(position);

    if (value)
    {
        if (ci != CellInfo::Unknown)
        {
            return;
        }

        ci = CellInfo::MarkedLandmine;
        ++landmines_marked_;
    }
    else
    {
        if (ci != CellInfo::MarkedLandmine)
        {
            return;
        }

        ci = CellInfo::Unknown;
        --landmines_marked_;
    }
}

void GameBoard::dump_region(qed::FieldPosition poi, index_type range) const
{
    std::cout << "center=" << poi << "\n";
    index_type column_0 = poi.column > range + 1 ? poi.column - range - 1 : 0;
    index_type column_1 = std::min(columns() - 1, poi.column + range + 1);
    std::cout << "columns: [" << column_0 << " .. " << column_1 << "]\n";
    for (index_type row = poi.row > range + 1 ? poi.row - range - 1 : 0;
         row <= std::min(rows() - 1, poi.row + range + 1);
         ++row)
    {

        std::cout << row << ": ";
        for (index_type col = column_0; col <= column_1; ++col)
        {
            qed::FieldPosition position{row, col};
            char ch{};
            auto cell_info = at(position);
            switch (cell_info)
            {
            case CellInfo::Exploded:
                ch = '!';
                break;

            case GameBoard::CellInfo::MarkedLandmine:
                if (field_->is_landmine(position))
                {
                    ch = '*';
                }
                else
                    ch = '%';
                break;

            case GameBoard::CellInfo::Unknown:
                ch = '?';
                break;

            case GameBoard::CellInfo::N0:
            case GameBoard::CellInfo::N1:
            case GameBoard::CellInfo::N2:
            case GameBoard::CellInfo::N3:
            case GameBoard::CellInfo::N4:
            case GameBoard::CellInfo::N5:
            case GameBoard::CellInfo::N6:
            case GameBoard::CellInfo::N7:
            case GameBoard::CellInfo::N8:
                ch = '0' + static_cast<int>(cell_info);
                break;
            };
            std::cout << ch;
        }
        std::cout << "\n";
    }
}

} // namespace qed
