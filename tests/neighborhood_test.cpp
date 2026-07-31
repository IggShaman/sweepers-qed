#include "neighborhood.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <vector>

using field_position = qed::field_position_type<int32_t>;

static std::vector<field_position> collect(field_position center, int rows, int cols, int side)
{
    std::vector<field_position> v;
    for (auto p : qed::neighborhood(center, rows, cols, side))
    {
        v.push_back(p);
    }
    return v;
}

TEST_CASE("interior cell yields 9 neighbours at side 1")
{
    const auto v = collect({5, 5}, 10, 10, 1);
    REQUIRE(v.size() == 9);
    REQUIRE(
      std::any_of(v.begin(), v.end(), [](field_position p) { return p == field_position{5, 5}; }));
    REQUIRE(v[0] == field_position{4, 4});
    REQUIRE(v.back() == field_position{6, 6});
}

TEST_CASE("corners and edges are clipped to the board")
{
    REQUIRE(collect({0, 0}, 10, 10, 1).size() == 4);
    REQUIRE(collect({9, 9}, 10, 10, 1).size() == 4);
    REQUIRE(collect({0, 5}, 10, 10, 1).size() == 6);
    REQUIRE(collect({9, 5}, 10, 10, 1).size() == 6);
    REQUIRE(collect({5, 0}, 10, 10, 1).size() == 6);
    REQUIRE(collect({5, 9}, 10, 10, 1).size() == 6);
}

TEST_CASE("side 7 is the solver's 15x15 window")
{
    REQUIRE(collect({20, 20}, 100, 100, 7).size() == 15 * 15);
}

TEST_CASE("a 1x1 board has neighbourhood of 1")
{
    REQUIRE(collect({0, 0}, 1, 1, 1).size() == 1);
}
