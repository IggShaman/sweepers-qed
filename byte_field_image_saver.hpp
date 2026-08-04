#pragma once

#include <expected>
#include <string>

namespace qed
{
class byte_field;

std::expected<void, std::string> export_field_to_png(byte_field* field, std::string file_name);
} // namespace qed
