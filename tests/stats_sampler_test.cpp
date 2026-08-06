#include "solver_stats.hpp"
#include "stats_sampler.hpp"

#include <catch2/catch_test_macros.hpp>

namespace t
{
struct test_sample
{
    int run_id;
    std::chrono::milliseconds at;
    int int_value;

    test_sample& operator+=(const test_sample& other)
    {
        int_value += other.int_value;
        return *this;
    }
};

class test_solver_stats_provider : public qed::solver_stats_provider<test_sample>
{
    std::expected<test_sample, std::string> take_stats_sample() override
    {
        return test_sample{
          .int_value = ++int_value_,
        };
    }

private:
    int int_value_{0};
};
} // namespace t

TEST_CASE("stats_sampler basics")
{
    t::test_solver_stats_provider stats_provider;

    qed::stats_sampler<t::test_sample> sampler(1, &stats_provider, std::chrono::milliseconds{10});

    sampler.start();

    const int expected_samples_count = 15;
    std::this_thread::sleep_for(std::chrono::milliseconds{expected_samples_count * 10});

    auto [samples, runtime] = sampler.stop_and_take();
    REQUIRE(abs(static_cast<int>(samples.size() - expected_samples_count)) < 3);
    REQUIRE(runtime.count() >= 150);
    REQUIRE(runtime.count() <= 200);
    REQUIRE(samples[0].at.count() <= 20);
    REQUIRE(samples[1].at.count() <= 30);
    REQUIRE(samples[2].at.count() <= 40);
    REQUIRE(samples.back().at.count() <= 200);
    REQUIRE(samples.back().at.count() >= 150);
    for (auto& sample : samples)
    {
        REQUIRE(sample.run_id == 1);
    }
}
