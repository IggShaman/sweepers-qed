#pragma once

#include "board.hpp"
#include "field.hpp"

#include <QtWidgets/QWidget>

#include <cstddef>
#include <memory>

namespace sweeper
{

class GameBoardWidget : public QWidget {
    Q_OBJECT;
    
public:
    static constexpr int kCellSize = 20; // in pixels
    static constexpr float kScaleStep = 0.05;
    static constexpr float kMaxScale = 10.0;

    static constexpr int kPointModeScaleStep = 1;
    static constexpr int kDrawBorderScaleStep = 0.5 / kScaleStep;
    static constexpr int kDrawTextScaleStep = 0.2 / kScaleStep;
    static constexpr int kMinScaleStep = 1;
    static constexpr int kMaxScaleStep = kMaxScale / kScaleStep;

    GameBoardWidget();

    qed::GameBoardPtr board() { return board_; }
    void set_board(qed::GameBoardPtr);
    void set_show_landmines(bool);
    void update_cell(qed::FieldPosition);
    void update_box(qed::FieldPosition);
    void set_scale_step(int step);
    void set_rw(bool);

public slots:
    void zoom_in();
    void zoom_out();
    // Sets minimal zoom, which uses individual pixes to draw field
    void switch_point_mode(bool);
    
signals:
    void cell_changed(qed::FieldPosition);
    void game_lost();
    
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    bool is_point_mode() const { return scale_step_ == kPointModeScaleStep; }
    
private:
    void paint_cell(QPainter&, qed::FieldPosition);
    void paint_point_cell(QPainter&, qed::FieldPosition);
    qed::index_type x2column(size_t x)
    {
        return static_cast<qed::index_type>(
          is_point_mode() ? x : x / get_scale_factor() / kCellSize);
    }
    qed::index_type y2row(size_t y)
    {
        return static_cast<qed::index_type>(
          is_point_mode() ? y : y / get_scale_factor() / kCellSize);
    }
    size_t row2y(qed::index_type row)
    {
        return is_point_mode() ? static_cast<size_t>(row)
                               : get_scale_factor() * static_cast<size_t>(row) * kCellSize;
    }
    size_t column2x(size_t column)
    {
        return is_point_mode() ? static_cast<size_t>(column)
                               : get_scale_factor() * static_cast<size_t>(column) * kCellSize;
    }
    float get_scale_factor() const { return scale_step_ * kScaleStep; }
    int scaled_cell_size() const;
    void update_widget_size();

    qed::GameBoardPtr board_;
    bool show_landmines_{};
    bool rw_{};
    
    QColor cell_border_;
    QColor cell_opened_bg_;
    QColor cell_unknown_bg_;
    QFont cell_font_;
    QColor per_count_colors_text_[9];
    QColor per_count_colors_box_[9];
    std::size_t scale_step_ = 20;
    std::size_t prev_scale_step_ = 20; // go back to this when toggling scale mode
};

} // namespace sweeper
