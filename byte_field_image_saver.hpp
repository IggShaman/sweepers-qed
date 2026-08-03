#pragma once

#include <string>

namespace qed
{
class byte_field;

void export_field_to_png(byte_field* field, std::string file_name);
} // namespace qed
