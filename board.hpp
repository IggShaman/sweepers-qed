#pragma once

#include "field.hpp"

namespace landmine {

class CellNeighborhoodIterator;

class GameBoard {
public:
    enum class CellInfo : int8_t {
	Exploded = -3,
	MarkedMine = -2,
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
    
    void set_field(FieldPtr);
    CellInfo at(FieldPosition l) const { return data_[to_index(l)]; }
    void mark_mine(FieldPosition, bool);
    void mark_exploded(FieldPosition l) { edit_at(l) = CellInfo::Exploded; }
    void uncovered_safe(FieldPosition, uint8_t);
    size_t rows() const { return field_->rows(); }
    size_t cols() const { return field_->cols(); }
    size_t landmines_marked() const { return landmines_marked_; }
    FieldCPtr field() const { return field_; }
    CellNeighborhoodIterator neighborhood(FieldPosition);
    bool is_uncovered(FieldPosition l) const { return static_cast<int>(at(l)) >= 0; }
    bool game_lost() const { return game_lost_; }
    void set_game_lost() { game_lost_ = true; }
    size_t uncovered_count() const { return uncovered_count_; }
    size_t left_count() const { return data_.size() - uncovered_count_ - landmines_marked_; }
    void dump_region(FieldPosition, size_t range) const;

private:
    size_t to_index(const FieldPosition& l) const { return field_->cols() * l.row + l.col; }
    CellInfo& edit_at(FieldPosition l) { return data_[to_index(l)]; }

    FieldPtr field_;
    std::vector<CellInfo> data_;
    size_t landmines_marked_{};
    size_t uncovered_count_{};
    bool game_lost_{};
};

using GameBoardPtr = std::shared_ptr<GameBoard>;

class CellNeighborhoodIterator {
public:
    CellNeighborhoodIterator(GameBoard*, FieldPosition);

    CellNeighborhoodIterator& operator++() { ++i_; return *this; }
    operator bool() const { return i_ < end_; }
    GameBoard::CellInfo at() { return board_->at(neighbors_[i_]); }
    const FieldPosition& operator*() const { return neighbors_[i_]; }

private:
    uint8_t i_{}, end_{};
    std::array<FieldPosition, 8> neighbors_;
    GameBoard* board_{};
};

inline CellNeighborhoodIterator GameBoard::neighborhood(FieldPosition l) {
    return CellNeighborhoodIterator(this, l);
}

} // namespace landmine
