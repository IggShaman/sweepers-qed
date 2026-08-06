#pragma once

#include <expected>
#include <string>

namespace qed
{
template <typename Sample> class solver_stats_provider
{
public:
    virtual std::expected<Sample, std::string> take_stats_sample() = 0;
};
} // namespace qed
