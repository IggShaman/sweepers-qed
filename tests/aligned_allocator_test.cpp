#include "aligned_allocator.hpp"
#include "aligned_bytes.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("newly allocated data is aligned properly")
{
    REQUIRE(reinterpret_cast<std::uintptr_t>(i::aligned_bytes<8>(1024).data()) % 8 == 0);

    REQUIRE(reinterpret_cast<std::uintptr_t>(i::aligned_bytes<64>(1024).data()) % 64 == 0);

    REQUIRE(reinterpret_cast<std::uintptr_t>(i::aligned_bytes<128>(1024).data()) % 128 == 0);
}
