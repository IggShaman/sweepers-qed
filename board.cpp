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
    data_.resize(field_->rows() * field_->cols());
    std::fill(data_.begin(), data_.end(), CellInfo::Unknown);
}

CellNeighborhoodIterator::CellNeighborhoodIterator(GameBoard* board, qed::FieldPosition position)
    : board_{board}
{

    end_ = 0;
    if (position.row > 0)
    {
        if (position.col > 0)
        {
            neighbors_[end_++] = {position.row - 1, position.col - 1};
        }

        neighbors_[end_++] = {position.row - 1, position.col};

        if (position.col < board_->cols() - 1)
        {
            neighbors_[end_++] = {position.row - 1, position.col + 1};
        }
    }

    if (position.col > 0)
    {
        neighbors_[end_++] = {position.row, position.col - 1};
    }

    if (position.col < board_->cols() - 1)
    {
        neighbors_[end_++] = {position.row, position.col + 1};
    }

    if (position.row < board_->rows() - 1)
    {
        if (position.col > 0)
        {
            neighbors_[end_++] = {position.row + 1, position.col - 1};
        }

        neighbors_[end_++] = {position.row + 1, position.col};

        if (position.col < board_->cols() - 1)
        {
            neighbors_[end_++] = {position.row + 1, position.col + 1};
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

void GameBoard::dump_region(qed::FieldPosition poi, size_t range) const
{
    std::cout << "center=" << poi << "\n";
    size_t col0 = poi.col > range + 1 ? poi.col - range - 1 : 0;
    size_t col1 = std::min(cols() - 1, poi.col + range + 1);
    std::cout << "columns: [" << col0 << " .. " << col1 << "]\n";
    for (size_t row = poi.row > range + 1 ? poi.row - range - 1 : 0;
         row <= std::min(rows() - 1, poi.row + range + 1); ++row) {

        std::cout << row << ": ";
        for (size_t col = col0; col <= col1; ++col) {
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
