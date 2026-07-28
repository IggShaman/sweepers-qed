#pragma once

#include "field.hpp"

namespace qed
{

class CellNeighborhoodIterator;

class GameBoard {
public:
    enum class CellInfo : int8_t
    {
        Exploded = -3,
        MarkedLandmine = -2,
        Unknown = -1,
        N0 = 0,
        N1 = 1,
        N2 = 2,
        N3 = 3,
        N4 = 4,
        N5 = 5,
        N6 = 6,
        N7 = 7,
        N8 = 8,
    };

    void set_field(qed::FieldPtr);
    CellInfo at(qed::FieldPosition position) const { return data_[to_index(position)]; }
    void mark_mine(qed::FieldPosition, bool);
    void mark_exploded(qed::FieldPosition position) { edit_at(position) = CellInfo::Exploded; }
    void uncovered_safe(qed::FieldPosition, uint8_t nearby_landmines_count);
    index_type rows() const { return field_->rows(); }
    index_type columns() const { return field_->columns(); }
    index_type landmines_marked() const { return landmines_marked_; }
    qed::FieldCPtr field() const { return field_; }
    CellNeighborhoodIterator neighborhood(qed::FieldPosition);
    bool is_uncovered(qed::FieldPosition position) const
    {
        return static_cast<int>(at(position)) >= 0;
    }
    bool game_lost() const { return game_lost_; }
    void set_game_lost() { game_lost_ = true; }
    size_t uncovered_count() const { return uncovered_count_; }
    size_t left_count() const { return data_.size() - uncovered_count_ - landmines_marked_; }
    void dump_region(qed::FieldPosition, index_type range) const;

private:
    size_t to_index(const qed::FieldPosition& position) const
    {
        return field_->columns() * position.row + position.column;
    }
    CellInfo& edit_at(qed::FieldPosition position) { return data_[to_index(position)]; }

    qed::FieldPtr field_;
    std::vector<CellInfo> data_;
    size_t landmines_marked_{};
    size_t uncovered_count_{};
    bool game_lost_{};
};

using GameBoardPtr = std::shared_ptr<GameBoard>;

class CellNeighborhoodIterator {
public:
    CellNeighborhoodIterator(GameBoard*, qed::FieldPosition);

    CellNeighborhoodIterator& operator++() { ++i_; return *this; }
    operator bool() const { return i_ < end_; }
    GameBoard::CellInfo at() { return board_->at(neighbors_[i_]); }
    const qed::FieldPosition& operator*() const { return neighbors_[i_]; }

private:
    uint8_t i_{};
    uint8_t end_{};
    std::array<qed::FieldPosition, 8> neighbors_;
    GameBoard* board_{};
};

inline CellNeighborhoodIterator GameBoard::neighborhood(qed::FieldPosition position)
{
    return CellNeighborhoodIterator(this, position);
}

} // namespace qed
