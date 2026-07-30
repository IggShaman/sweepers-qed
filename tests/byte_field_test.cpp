#include "byte_field.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iostream>

namespace qed
{
struct byte_field_access
{
protected:
    static auto& get_data(byte_field& f) { return f.data_; }
    static auto get_row_stride(byte_field& f) { return f.row_stride_; }
};
} // namespace qed

TEST_CASE_METHOD(qed::byte_field_access, "init")
{
    qed::byte_field f1;
    f1.reset(32, 16);
    REQUIRE(f1.rows() == 32);
    REQUIRE(f1.columns() == 16);

    auto* base_ptr = get_data(f1).data();
    REQUIRE(reinterpret_cast<std::uintptr_t>(base_ptr) % 128 == 0);
    REQUIRE(get_row_stride(f1) == 128);
}

TEST_CASE_METHOD(qed::byte_field_access, "init2")
{
    qed::byte_field f;
    f.reset(1, 128);
    REQUIRE(f.rows() == 1);
    REQUIRE(f.columns() == 128);

    REQUIRE(get_row_stride(f) == 128);
}

TEST_CASE_METHOD(qed::byte_field_access, "init3")
{
    qed::byte_field f;
    f.reset(2, 129);
    REQUIRE(f.rows() == 2);
    REQUIRE(f.columns() == 129);
    REQUIRE(get_row_stride(f) == 256);
}

TEST_CASE_METHOD(qed::byte_field_access, "cell")
{
    qed::byte_field f;
    f.reset(16, 16);

    {
        auto position = qed::field_position(0, 0);
        auto cell = f.cell_at(position);
        REQUIRE(cell.is_landmine_groundtruth() == false);
        REQUIRE(cell.is_uncovered() == false);
        REQUIRE(cell.is_marked_as_landmine() == false);
    }

    for (qed::index_type r = 0; r < 16; ++r)
    {
        for (qed::index_type c = 0; c < 16; ++c)
        {
            auto position = qed::field_position(r, c);
            auto cell = f.cell_at(position);
            if ((r + c) % 2 == 0)
            {
                cell.set_uncovered();
            }
            else
            {
                cell.set_marked_as_landmine();
                cell.set_landmine_groundtruth(true);
            }
        }
    }

    for (qed::index_type r = 0; r < 16; ++r)
    {
        for (qed::index_type c = 0; c < 16; ++c)
        {
            auto position = qed::field_position(r, c);
            auto cell = f.cell_at(position);
            if ((r + c) % 2 == 0)
            {
                REQUIRE(cell.is_uncovered() == true);
                REQUIRE(cell.is_marked_as_landmine() == false);
                REQUIRE(cell.is_landmine_groundtruth() == false);
            }
            else
            {
                REQUIRE(cell.is_uncovered() == false);
                REQUIRE(cell.is_marked_as_landmine() == true);
                REQUIRE(cell.is_landmine_groundtruth() == true);
            }
        }
    }

    // at this point, nearby mine counts are not cached
    for (qed::index_type r = 0; r < 16; ++r)
    {
        for (qed::index_type c = 0; c < 16; ++c)
        {
            auto ref = std::atomic_ref<uint8_t>(get_data(f)[f.index_at(qed::field_position(r, c))]);
            auto b = ref.load(std::memory_order::relaxed);
            REQUIRE((b & qed::byte_field_cell::kNearbyLandminesCountMask) == 0);
        }
    }

    // force all cells to compute and cache neighbor counts
    for (qed::index_type r = 0; r < 16; ++r)
    {
        for (qed::index_type c = 0; c < 16; ++c)
        {
            auto position = qed::field_position(r, c);
            auto cell = f.cell_at(position);

            auto ref = std::atomic_ref<uint8_t>(get_data(f)[f.index_at(position)]);
            {
                auto b = ref.load(std::memory_order::relaxed);
                REQUIRE((b >> 4) == 0);
            }

            if ((r + c) % 2 == 0)
            {
                uint8_t expected_count = 4;
                if (r == 0 or r == 15)
                {
                    --expected_count;
                }
                if (c == 0 or c == 15)
                {
                    --expected_count;
                }

                REQUIRE(f.nearby_landmines_count(position) == expected_count);
                {
                    auto b = ref.load(std::memory_order::relaxed);
                    REQUIRE((b >> 4) - 1 == expected_count);
                }
            }
        }
    }
}
