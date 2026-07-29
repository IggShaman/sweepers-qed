#pragma once

#include "aligned_allocator.hpp"

#include <cstdint>
#include <vector>

namespace i
{
template <std::size_t A>
using aligned_bytes = std::vector<std::uint8_t, aligned_allocator<std::uint8_t, A>>;
} // namespace i
