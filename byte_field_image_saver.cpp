#include "byte_field_image_saver.hpp"

#include "byte_field.hpp"
#include "logger.hpp"
#include "tests/byte_field_access.hpp"

#include <QImage>
#include <QList>
#include <QRgb>

namespace qed
{
// Creates a palette.
QList<QRgb> make_palette()
{
    QList<QRgb> palette(256);
    for (int value = 0; value < 256; ++value)
    {
        const bool is_gt_mine = value & qed::byte_field_cell::kGTMask;
        const bool is_uncovered = value & qed::byte_field_cell::kUncoveredMask;
        const bool is_marked = value & qed::byte_field_cell::kMarkedAsLandmineMask;
        // -1: not computed
        const int nearby_mines_count = (value >> 4) ? (value >> 4) - 1 : -1;

        if (is_marked)
        {
            palette[value] = qRgb(220, 60, 60);
        }
        else if (!is_uncovered)
        {
            palette[value] = qRgb(110, 110, 120);
        }
        else if (is_gt_mine)
        {
            palette[value] = qRgb(20, 20, 20);
        }
        else if (nearby_mines_count > 0)
        {
            const int g = 255 - nearby_mines_count * 24;
            palette[value] = qRgb(g, g, 255);
        }
        else
        {
            // uncovered, nearby landmines not computed
            palette[value] = qRgb(245, 245, 245);
        }
    }

    return palette;
}

std::expected<void, std::string> export_field_to_png(byte_field* field, std::string file_name)
{
    QString file_name_ = QString::fromUtf8(file_name);

    byte_field_access bfa;
    QImage img(
      byte_field_access::get_data(*field).data(),
      static_cast<int>(field->columns()),
      static_cast<int>(field->rows()),
      static_cast<int>(byte_field_access::get_row_stride(*field)),
      QImage::Format_Indexed8);
    img.setColorTable(make_palette());

    if (field->columns() < 256 or field->columns() < 256)
    {
        img = img.scaled(
          field->columns() * 8,
          field->rows() * 8,
          Qt::KeepAspectRatio,
          Qt::FastTransformation);
    }

    if (!img.save(file_name_))
    {
        return std::unexpected{I_TO_STRING("png save to \"" << file_name << "\" failed")};
    }

    return {};
}
} // namespace qed
