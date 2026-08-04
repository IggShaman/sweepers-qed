#include "game_board_widget.hpp"

#include <QPaintEvent>
#include <QPainter>

#include <algorithm>

#include "logger.hpp"

namespace sweeper
{
GameBoardWidget::GameBoardWidget()
    : field_{new qed::byte_field}, cell_border_{200, 200, 200}, cell_opened_bg_{220, 220, 220},
      cell_unknown_bg_{100, 100, 100},
      per_count_colors_text_{
        cell_opened_bg_,
        Qt::darkBlue,
        Qt::darkGreen,
        Qt::darkCyan,
        Qt::darkMagenta,
        Qt::black,
        Qt::black,
        Qt::black,
        Qt::black},
      per_count_colors_box_{
        cell_opened_bg_,
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
}

void GameBoardWidget::set_field(qed::byte_field_ptr new_field)
{
    field_ = new_field;
    update_widget_size();
}

void GameBoardWidget::update_widget_size()
{
    setFixedSize( //
      field_->columns() * scaled_cell_size(),
      field_->rows() * scaled_cell_size());
    update();
}

void GameBoardWidget::set_scale_step(int scale_step)
{
    scale_step = std::clamp(scale_step, kMinScaleStep, kMaxScaleStep);
    if (scale_step_ == scale_step)
    {
        return;
    }

    prev_scale_step_ = scale_step_;
    scale_step_ = scale_step;
    update_widget_size();
}

void GameBoardWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter{this};

    if (is_point_mode())
    {
        for (qed::index_type row = static_cast<qed::index_type>(std::max(0, event->rect().top()));
             row <= static_cast<qed::index_type>(std::max(0, event->rect().bottom()));
             ++row)
        {
            for (qed::index_type col =
                   static_cast<qed::index_type>(std::max(0, event->rect().left()));
                 col <= static_cast<qed::index_type>(std::max(0, event->rect().right()));
                 ++col)
            {
                paint_point_cell(painter, {row, col});
            }
        }
    }
    else
    {
        for (qed::index_type row =
               y2row(static_cast<qed::index_type>(std::max(0, event->rect().top())));
             row <= y2row(static_cast<qed::index_type>(std::max(0, event->rect().bottom())));
             ++row)
        {
            for (qed::index_type col =
                   x2column(static_cast<qed::index_type>(std::max(0, event->rect().left())));
                 col <= x2column(static_cast<qed::index_type>(std::max(0, event->rect().right())));
                 ++col)
            {
                paint_cell(painter, {row, col});
            }
        }
    }
}

void GameBoardWidget::paint_cell(QPainter& painter, qed::field_position position)
{
    painter.save();

    painter.translate(column2x(position.column), row2y(position.row));
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

    auto cell = field_->cell_at(position);
    if (cell.is_marked_as_landmine())
    {
        painter.fillRect(r, QBrush(Qt::red));
    }
    else if (cell.is_uncovered())
    {
        auto count = field_->nearby_landmines_count(position);

        if (scale_step_ >= kDrawTextScaleStep)
        {
            painter.fillRect(r, cell_opened_bg_);
            if (count)
            {
                painter.setPen(per_count_colors_text_[count]);
                painter.drawText(QRectF(r), Qt::AlignCenter, QString::number(count));
            }
        }
        else
        {
            painter.fillRect(r, per_count_colors_box_[count]);
        }
    }
    else // covered
    {
        painter.fillRect(r, cell_unknown_bg_);
    }

    if (show_landmines_ and field_->cell_at(position).is_landmine_groundtruth())
    {
        if (scale_step_ >= kDrawTextScaleStep)
        {
            painter.setPen(Qt::red);
            for (qed::index_type r = 1; r < 8; ++r)
            {
                // TODO??
                for (qed::index_type c = kCellSize - 8 + r; c < kCellSize; ++c)
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

void GameBoardWidget::paint_point_cell(QPainter& painter, qed::field_position position)
{
    auto cell = field_->cell_at(position);

    if (cell.is_marked_as_landmine())
    {
        painter.setPen(Qt::red);
    }
    else if (cell.is_uncovered())
    {
        auto count = field_->nearby_landmines_count(position);
        painter.setPen(count ? per_count_colors_box_[count] : cell_opened_bg_);
    }
    else // is_covered
    {
        if (show_landmines_ and cell.is_landmine_groundtruth())
        {
            painter.setPen(Qt::darkRed);
        }
        else
        {
            painter.setPen(cell_unknown_bg_);
        }
    }

    painter.drawPoint(position.column, position.row);
}

void GameBoardWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!rw_ or game_lost)
    {
        return;
    }

    const auto mouse_position = event->position().toPoint();

    qed::field_position field_position{
      static_cast<qed::index_type>(std::clamp(y2row(mouse_position.y()), 0, field_->rows() - 1)),
      static_cast<qed::index_type>(
        std::clamp(x2column(mouse_position.x()), 0, field_->columns() - 1))};

    switch (event->button())
    {
    case Qt::LeftButton: {
        // NOTE: no modifiers pressed
        auto cell = field_->cell_at(field_position);
        if (cell.is_uncovered())
        {
            return;
        }

        event->accept();
        cell.set_uncovered();
        if (cell.is_landmine_groundtruth())
        {
            game_lost = true;
            emit game_lost_signal();
        }
        else
        {
            emit new_poi(field_position);
        }

        update_cell(field_position);
        break;
    }

    case Qt::RightButton: {
        event->accept();
        auto cell = field_->cell_at(field_position);
        if (cell.is_uncovered())
        {
            break;
        }

        if (cell.is_marked_as_landmine())
        {
            cell.clear_marked_as_landmine();
            update_cell(field_position);
        }
        else
        {
            cell.set_marked_as_landmine();
            update_cell(field_position);
        }
        break;
    }

    default:
        break;
    };
}

void GameBoardWidget::update_cell(qed::field_position position)
{
    if (is_point_mode())
    {
        update(position.column, position.row, 1, 1);
    }
    else
    {
        update( //
          column2x(position.column),
          row2y(position.row),
          scaled_cell_size(),
          scaled_cell_size());
    }
}

void GameBoardWidget::update_box(qed::field_position center)
{
    auto size = is_point_mode() ? 1 : scaled_cell_size();
    update(center.column, center.row, size, size);
}

void GameBoardWidget::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        event->accept();

        auto steps = event->angleDelta() / 8 / 15;
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

int GameBoardWidget::scaled_cell_size() const
{
    return is_point_mode() ? 1 : get_scale_factor() * kCellSize;
}

void GameBoardWidget::set_show_landmines(bool value)
{
    show_landmines_ = value;
    update();
}

void GameBoardWidget::set_rw(bool value)
{
    rw_ = value;
}
} // namespace sweeper
