#pragma once

#include "field_config.hpp"

#include <expected>
#include <string>

namespace qed
{
std::expected<std::vector<field_config>, std::string> load_field_configs(std::string file_name);

std::expected<void, std::string>
save_field_configs(std::string file_name, const std::vector<field_config>&);
} // namespace qed
