#pragma once

#include <chrono>

namespace b
{
struct scoped_timer
{
    using ms = std::chrono::duration<double, std::milli>;

    ms tdiff() const { return std::chrono::steady_clock::now() - t0; }

    std::chrono::steady_clock::time_point t0{std::chrono::steady_clock::now()};
};
} // namespace b
