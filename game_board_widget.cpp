#include "game_board_widget.hpp"
#include "board.hpp"

#include <QPaintEvent>
#include <QPainter>

namespace sweeper
{

size_t subtract_floor_0(size_t a, size_t b) {
    return a > b ? a - b : 0;
}

GameBoardWidget::GameBoardWidget()
    : board_{new qed::GameBoard}, cell_border_{200, 200, 200}, cell_opened_bg_{220, 220, 220},
      cell_unknown_bg_{100, 100, 100},
      per_count_colors_text_{
        Qt::black,
        Qt::darkBlue,
        Qt::darkGreen,
        Qt::darkCyan,
        Qt::darkMagenta,
        Qt::black,
        Qt::black,
        Qt::black},
      per_count_colors_box_{
        Qt::black,
        Qt::blue,
        Qt::green,
        Qt::cyan,
        Qt::magenta,
        Qt::yellow,
        Qt::yellow,
        Qt::yellow}
{
    cell_font_.setPixelSize(kCellSize - 4);
    cell_font_.setBold(true);
    board_->set_field(std::make_shared<qed::Field>());
}

void GameBoardWidget::set_board(qed::GameBoardPtr b)
{
    board_ = b;
    update_widget_size();
}

void GameBoardWidget::update_widget_size() {
    setFixedSize(board_->cols() * scaled_cell_size(), board_->rows() * scaled_cell_size());
    update();
}

void GameBoardWidget::set_scale_step(size_t s) {
    if (s < kMinScaleStep) {
        s = kMinScaleStep;
    } else if (s > kMaxScaleStep) {
        s = kMaxScaleStep;
    }

    if (scale_step_ != s) {
        prev_scale_step_ = scale_step_;
        scale_step_ = s;
        update_widget_size();
    }
}

void GameBoardWidget::paintEvent(QPaintEvent* ev) {
    QPainter painter{this};

    if (is_point_mode()) {
        for (size_t row = (size_t)std::max(0, ev->rect().top());
             row <= (size_t)std::max(0, ev->rect().bottom()); ++row) {
            for (size_t col = (size_t)std::max(0, ev->rect().left());
                 col <= (size_t)std::max(0, ev->rect().right()); ++col) {
                paint_point_cell(painter, {row, col});
            }
        }
    } else {
        for (size_t row = y2row((size_t)std::max(0, ev->rect().top()));
             row <= y2row((size_t)std::max(0, ev->rect().bottom())); ++row) {
            for (size_t col = x2col((size_t)std::max(0, ev->rect().left()));
                 col <= x2col((size_t)std::max(0, ev->rect().right())); ++col) {
                paint_cell(painter, {row, col});
            }
        }
    }
}

void GameBoardWidget::paint_cell(QPainter& painter, qed::FieldPosition position)
{
    painter.save();

    painter.translate(col2x(position.col), row2y(position.row));
    auto scale_factor = get_scale_factor();
    painter.scale(scale_factor, scale_factor);

    QRect r;
    if (scale_step_ >= kDrawBorderScaleStep)
    {
        // draw border
        painter.setPen(cell_border_);
        painter.drawLine(0, 0, kCellSize - 1, 0);
        painter.drawLine(0, 0, 0, kCellSize - 1);
        r = {1, 1, kCellSize - 1, kCellSize - 1};
    }
    else
    {
        // no border
        r = {0, 0, kCellSize, kCellSize};
    }

    if (scale_step_ >= kDrawTextScaleStep)
    {
        painter.setFont(cell_font_);
    }

    auto cell_info = board_->at(position);
    switch (cell_info)
    {
    case qed::GameBoard::CellInfo::Exploded:
        painter.fillRect(r, QBrush(Qt::black));
        break;

    case qed::GameBoard::CellInfo::MarkedLandmine:
        painter.fillRect(r, QBrush(Qt::red));
        break;

    case qed::GameBoard::CellInfo::Unknown:
        painter.fillRect(r, cell_unknown_bg_);
        break;

    case qed::GameBoard::CellInfo::N0:
        break;

    case qed::GameBoard::CellInfo::N1:
    case qed::GameBoard::CellInfo::N2:
    case qed::GameBoard::CellInfo::N3:
    case qed::GameBoard::CellInfo::N4:
    case qed::GameBoard::CellInfo::N5:
    case qed::GameBoard::CellInfo::N6:
    case qed::GameBoard::CellInfo::N7:
    case qed::GameBoard::CellInfo::N8:
        if (scale_step_ >= kDrawTextScaleStep)
        {
            painter.fillRect(r, cell_opened_bg_);
            painter.setPen(per_count_colors_text_[static_cast<int>(cell_info)]);
            painter.drawText(1, kCellSize - 1, QString::number(static_cast<int>(cell_info)));
        }
        else
        {
            painter.fillRect(r, per_count_colors_box_[static_cast<int>(cell_info)]);
        }
        break;
    };

    if (show_landmines_ and board_->field()->is_landmine(position))
    {
        if (scale_step_ >= kDrawTextScaleStep)
        {
            painter.setPen(Qt::red);
            for (size_t r = 1; r < 8; ++r)
            {
                for (size_t c = kCellSize - 8 + r; c < kCellSize; ++c)
                {
                    painter.drawPoint(c, r);
                }
            }
        }
        else
        {
            painter.fillRect(r, Qt::darkRed);
        }
    }

    painter.restore();
}

void GameBoardWidget::paint_point_cell(QPainter& painter, qed::FieldPosition position)
{
    auto cell_info = board_->at(position);
    switch (cell_info)
    {
    case qed::GameBoard::CellInfo::Exploded:
        painter.setPen(Qt::black);
        break;

    case qed::GameBoard::CellInfo::MarkedLandmine:
        painter.setPen(Qt::red);
        break;

    case qed::GameBoard::CellInfo::Unknown:
        if (show_landmines_ and board_->field()->is_landmine(position))
        {
            painter.setPen(Qt::darkRed);
        }
        else
            painter.setPen(cell_unknown_bg_);
        break;

    case qed::GameBoard::CellInfo::N0:
        painter.setPen(cell_opened_bg_);
        break;

    case qed::GameBoard::CellInfo::N1:
    case qed::GameBoard::CellInfo::N2:
    case qed::GameBoard::CellInfo::N3:
    case qed::GameBoard::CellInfo::N4:
    case qed::GameBoard::CellInfo::N5:
    case qed::GameBoard::CellInfo::N6:
    case qed::GameBoard::CellInfo::N7:
    case qed::GameBoard::CellInfo::N8:
        painter.setPen(per_count_colors_box_[static_cast<int>(cell_info)]);
        break;
    };

    painter.drawPoint(position.col, position.row);
}

void GameBoardWidget::mouseReleaseEvent(QMouseEvent* ev) {
    if (!rw_ or board_->game_lost())
        return;

    const auto pos = ev->position().toPoint();

    qed::FieldPosition position;
    if (is_point_mode())
    {
        position = {
          //
          static_cast<size_t>(std::max(0, pos.y())),
          static_cast<size_t>(std::max(0, pos.x()))};
    }
    else
    {
        position = {
          //
          y2row(static_cast<size_t>(std::max(0, pos.y()))),
          x2col(static_cast<size_t>(std::max(0, pos.x())))};
    }

    switch (ev->button())
    {
    case Qt::LeftButton: {
        // NOTE: no modifiers pressed
        if (board_->at(position) != qed::GameBoard::CellInfo::Unknown)
        {
            return;
        }

        ev->accept();
        if (board_->field()->is_landmine(position))
        {
            board_->mark_exploded(position);
            emit cell_changed(position);
            emit game_lost();
        }
        else
        {
            board_->uncovered_safe(position, board_->field()->nearby_landmines_count(position));
            emit cell_changed(position);
        }

        update_cell(position);
        break;
    }

    case Qt::RightButton: {
        ev->accept();
        switch (board_->at(position))
        {
        case qed::GameBoard::CellInfo::MarkedLandmine:
            board_->mark_mine(position, false);
            update_cell(position);
            emit cell_changed(position);
            break;

        case qed::GameBoard::CellInfo::Unknown:
            board_->mark_mine(position, true);
            update_cell(position);
            emit cell_changed(position);
            break;

        default:
            break;
        };
        break;
    }

    default:
        break;
    };
}

void GameBoardWidget::update_cell(qed::FieldPosition position)
{
    if (is_point_mode())
    {
        update(position.col, position.row, 1, 1);
    }
    else
    {
        update( //
          col2x(position.col),
          row2y(position.row),
          scaled_cell_size(),
          scaled_cell_size());
    }
}

void GameBoardWidget::update_box(qed::FieldPosition center, size_t range)
{
    if (is_point_mode())
    {
        update(
          subtract_floor_0(center.col, range),
          subtract_floor_0(center.row, range),
          1 + range * 2,
          1 + range * 2);
    }
    else
    {
        update(
          col2x(subtract_floor_0(center.col, range)),
          row2y(subtract_floor_0(center.row, range)),
          (1 + range * 2) * scaled_cell_size(),
          (1 + range * 2) * scaled_cell_size());
    }
}

void GameBoardWidget::wheelEvent(QWheelEvent* ev) {
    if (ev->modifiers() & Qt::ControlModifier) {
        ev->accept();

        auto steps = ev->angleDelta() / 8 / 15;
        if (steps.isNull())
            return;

        set_scale_step((size_t)std::max(0, steps.y() + (int)scale_step_));
        return;
    }
}

void GameBoardWidget::zoom_out() {
    set_scale_step(scale_step_ - 1);
}

void GameBoardWidget::zoom_in() {
    set_scale_step(scale_step_ + 1);
}

void GameBoardWidget::switch_point_mode(bool v) {
    set_scale_step(v ? kPointModeScaleStep : prev_scale_step_);
}

size_t GameBoardWidget::scaled_cell_size() const {
    return is_point_mode() ? 1 : get_scale_factor() * kCellSize;
}

} // namespace sweeper
