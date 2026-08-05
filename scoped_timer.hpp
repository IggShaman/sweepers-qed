#pragma once

#include <chrono>

namespace b
{
using ms_duration_type = std::chrono::duration<double, std::milli>;

struct scoped_timer
{
    ms_duration_type tdiff() const { return std::chrono::steady_clock::now() - t0; }

    std::chrono::steady_clock::time_point t0{std::chrono::steady_clock::now()};
};
} // namespace b
