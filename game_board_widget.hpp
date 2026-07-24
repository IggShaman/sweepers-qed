#pragma once

#include "field.hpp"

#include <QtWidgets/QWidget>

#include <cstddef>
#include <memory>

namespace landmine {

class GameBoard;
using GameBoardPtr = std::shared_ptr<GameBoard>;

class GameBoardWidget : public QWidget {
    Q_OBJECT;
    
public:
    static constexpr std::size_t kCellSize = 20; // in pixels
    static constexpr float kScaleStep = 0.05;
    static constexpr float kMaxScale = 10.0;

    static constexpr std::size_t kPointModeScaleStep = 1;
    static constexpr std::size_t kDrawBorderScaleStep = 0.5 / kScaleStep;
    static constexpr std::size_t kDrawTextScaleStep = 0.2 / kScaleStep;
    static constexpr std::size_t kMinScaleStep = 1;
    static constexpr std::size_t kMaxScaleStep = kMaxScale / kScaleStep;
    
    GameBoardWidget();
    
    GameBoardPtr board() { return board_; }
    void set_board(GameBoardPtr);
    void set_show_mines(bool v) { show_mines_ = v; update(); }
    void update_cell(FieldPosition);
    void update_box(FieldPosition center, std::size_t range);
    void set_scale_step(std::size_t step);
    void set_rw(bool v) { rw_ = v; }

public slots:
    void zoom_in();
    void zoom_out();
    // Sets minimal zoom, which uses individual pixes to draw field
    void switch_point_mode(bool);
    
signals:
    void cell_changed(landmine::FieldPosition);
    void game_lost();
    
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    bool is_point_mode() const { return scale_step_ == kPointModeScaleStep; }
    
private:
    void paint_cell(QPainter&, FieldPosition);
    void paint_point_cell(QPainter&, FieldPosition);
    std::size_t x2col(std::size_t x) { return is_point_mode() ? 1 : x / get_scale_factor() / kCellSize; }
    std::size_t y2row(std::size_t y) { return is_point_mode() ? 1 : y / get_scale_factor() / kCellSize; }
    std::size_t row2y(std::size_t row) { return is_point_mode() ? 1 : get_scale_factor() * row * kCellSize; }
    std::size_t col2x(std::size_t col) { return is_point_mode() ? 1 : get_scale_factor() * col * kCellSize; }
    float get_scale_factor() const { return scale_step_ * kScaleStep; }
    std::size_t scaled_cell_size() const;
    void update_widget_size();
    
    GameBoardPtr board_;
    bool show_mines_{};
    bool rw_{};
    
    QColor cell_border_;
    QColor cell_opened_bg_;
    QColor cell_unknown_bg_;
    QFont cell_font_;
    QColor per_nr_colors_text_[8];
    QColor per_nr_colors_box_[8];
    std::size_t scale_step_ = 20;
    std::size_t prev_scale_step_ = 20; // go back to this when toggling scale mode
};

} // namespace landmine
