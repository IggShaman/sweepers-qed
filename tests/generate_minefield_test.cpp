#include "byte_field.hpp"
#include "minefield_generator.hpp"

#include <catch2/catch_test_macros.hpp>

namespace qed
{
struct byte_field_access
{
protected:
    static auto& get_data(byte_field& f) { return f.data_; }
    static auto get_row_stride(byte_field& f) { return f.row_stride_; }
};

size_t count_landmines(auto* field)
{
    size_t count{};
    for (qed::index_type j = 0; j < field->rows(); ++j)
    {
        for (qed::index_type i = 0; i < field->columns(); ++i)
        {
            auto cell = field->cell_at({j, i});
            if (cell.is_landmine_groundtruth())
            {
                ++count;
            }
        }
    }

    return count;
}
} // namespace qed

TEST_CASE_METHOD(qed::byte_field_access, "3x3 / 2")
{
    qed::byte_field f;
    f.reset(3, 3);
    REQUIRE(f.rows() == 3);
    REQUIRE(f.columns() == 3);

    REQUIRE(qed::generate_minefield(&f, 2) == true);
    REQUIRE(qed::count_landmines(&f) == 2);
}

TEST_CASE_METHOD(qed::byte_field_access, "3x3 / 6")
{
    qed::byte_field f;
    f.reset(3, 3);
    REQUIRE(qed::generate_minefield(&f, 6) == true);
    REQUIRE(qed::count_landmines(&f) == 6);
}

TEST_CASE_METHOD(qed::byte_field_access, "3x3 / 9")
{
    qed::byte_field f;
    f.reset(3, 3);
    REQUIRE(qed::generate_minefield(&f, 9) == false);
}

TEST_CASE_METHOD(qed::byte_field_access, "512x8 / 2048")
{
    qed::byte_field f;
    f.reset(512, 8);
    REQUIRE(qed::generate_minefield(&f, 2048) == true);
    REQUIRE(qed::count_landmines(&f) == 2048);
}
