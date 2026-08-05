#include "byte_field.hpp"
#include "byte_field_access.hpp"
#include "field_stats.hpp"
#include "minefield_generator.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(qed::byte_field_access, "3x3 / 2")
{
    qed::byte_field f;
    f.reset(3, 3);
    REQUIRE(f.rows() == 3);
    REQUIRE(f.columns() == 3);

    REQUIRE(qed::generate_minefield(&f, 2));
    REQUIRE(b::count_field_stats(&f).landmines == 2);
}

TEST_CASE_METHOD(qed::byte_field_access, "3x3 / 6")
{
    qed::byte_field f;
    f.reset(3, 3);
    REQUIRE(qed::generate_minefield(&f, 6));
    REQUIRE(b::count_field_stats(&f).landmines == 6);
}

TEST_CASE_METHOD(qed::byte_field_access, "3x3 / 9")
{
    qed::byte_field f;
    f.reset(3, 3);
    REQUIRE(!qed::generate_minefield(&f, 9));
}

TEST_CASE_METHOD(qed::byte_field_access, "512x8 / 2048")
{
    qed::byte_field f;
    f.reset(512, 8);
    REQUIRE(qed::generate_minefield(&f, 2048));
    REQUIRE(b::count_field_stats(&f).landmines == 2048);
}
