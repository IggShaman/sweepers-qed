#pragma once

#include <chrono>

namespace qed
{
using ms_duration_type = std::chrono::duration<double, std::milli>;

struct scoped_timer
{
    std::chrono::milliseconds get_elapsed_ms() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - t0);
    }

    std::chrono::steady_clock::time_point t0{std::chrono::steady_clock::now()};
};
} // namespace qed
