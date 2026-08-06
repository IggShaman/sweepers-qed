#pragma once

#include "logger.hpp"
#include "solver_stats.hpp"

#include <chrono>
#include <thread>
#include <vector>

namespace qed
{
template <typename Sample> class stats_sampler
{
public:
    explicit stats_sampler(solver_stats<Sample>* stats_source, std::chrono::milliseconds interval)
        : stats_source_{stats_source}, interval_{interval}
    {
    }

    void start();
    std::tuple<std::vector<Sample>, std::chrono::milliseconds> stop_and_take();

private:
    void run(std::stop_token);
    void save_sample(uint32_t skipped_count);

    std::vector<Sample> samples_;

    solver_stats<Sample> stats_source_{};
    std::chrono::milliseconds interval_{10};
    std::jthread thread_;
    std::chrono::steady_clock::time_point t0_;
};

template <typename Sample> void stats_sampler<Sample>::start()
{
    t0_ = std::chrono::steady_clock::now();
    thread_ = std::jthread{[this](std::stop_token stop_token) { run(stop_token); }};
}

template <typename Sample>
std::tuple<std::vector<Sample>, std::chrono::milliseconds> stats_sampler<Sample>::stop_and_take()
{
    thread_.request_stop();
    if (thread_.joinable())
    {
        thread_.join();
    }
    return {std::move(samples_), std::chrono::steady_clock::now() - t0_};
}

template <typename Sample> void stats_sampler<Sample>::run(std::stop_token stop_token)
{
    auto next_at = t0_;
    while (!stop_token.stop_requested())
    {
        next_at += interval_;

        uint32_t skipped_count{};
        const auto now = std::chrono::steady_clock::now();
        while (next_at <= now)
        {
            next_at += interval_;
            ++skipped_count;
        }

        std::this_thread::sleep_until(next_at);
        save_sample(skipped_count);
    }

    // save the final sample
    save_sample(0);
}

template <typename Sample> void stats_sampler<Sample>::save_sample(uint32_t skipped_samples)
{
    auto sample = stats_source_->take_stats_sample();
    if (!sample)
    {
        errlog << sample.error() << "\n";
        return;
    }

    if (skipped_samples)
    {
        errlog << SHOW(skipped_samples) << "\n";
    }

    sample->at = std::chrono::steady_clock::now() - t0_;
    samples_.push_back(*sample);
}
} // namespace qed
