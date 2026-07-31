#include "glpk_lp_problem.hpp"
#include "glpk_solver.hpp"

#include <catch2/catch_test_macros.hpp>
#include <unordered_set>

namespace qed
{
struct solver_access
{
protected:
    static std::deque<field_position>& get_poi(Solver& s) { return s.poi_; }
};

struct glpk_solver_access : public solver_access
{
protected:
    static void run_prepare_block(
      GlpkSolver& s,
      lp::problem& lp_problem,
      field_position pos,
      GlpkSolver::variables_map_type& vars)
    {
        s.prepare_block(lp_problem, pos, vars);
    }

    static bool
    run_test_block(GlpkSolver& s, lp::problem& lp_problem, GlpkSolver::variables_map_type& vars)
    {
        return s.test_block(lp_problem, vars);
    }
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

TEST_CASE_METHOD(qed::glpk_solver_access, "3x3 classic")
{
    auto field = std::make_shared<qed::byte_field>();
    field->reset(3, 3);

    for (auto pos : std::initializer_list<qed::field_position>{{0, 0}, {1, 0}})
    {
        field->cell_at(pos).set_landmine_groundtruth(pos);
    }

    qed::GlpkSolver solver(field);

    {
        qed::field_position c02{0, 2};
        auto cell02 = field->cell_at(c02);
        cell02.set_uncovered();
        REQUIRE(field->nearby_landmines_count(c02) == 0);
    }

    {
        lp::problem lp_problem;
        qed::GlpkSolver::variables_map_type vars;

        run_prepare_block(solver, lp_problem, {0, 2}, vars);

        REQUIRE(vars.size() == 3);
        REQUIRE(vars.contains({0, 1}) == true);
        REQUIRE(vars.contains({1, 1}) == true);
        REQUIRE(vars.contains({1, 2}) == true);

        run_test_block(solver, lp_problem, vars);

        auto cell01 = field->cell_at({0, 1});
        REQUIRE(cell01.is_uncovered());
        REQUIRE(field->nearby_landmines_count({0, 1}) == 2);

        auto cell11 = field->cell_at({1, 1});
        REQUIRE(cell11.is_uncovered());
        REQUIRE(field->nearby_landmines_count({1, 1}) == 2);

        auto cell12 = field->cell_at({1, 2});
        REQUIRE(cell12.is_uncovered());
        REQUIRE(field->nearby_landmines_count({1, 2}) == 0);
    }

    REQUIRE(
      (get_poi(solver) | std::ranges::to<std::unordered_set>()) ==
      std::unordered_set<qed::field_position>{{0, 1}, {1, 1}, {1, 2}});

    {
        lp::problem lp_problem;
        qed::GlpkSolver::variables_map_type vars;

        run_prepare_block(solver, lp_problem, {0, 1}, vars);

        REQUIRE(vars.size() == 5);
        REQUIRE(vars.contains({0, 0}) == true);
        REQUIRE(vars.contains({1, 0}) == true);
        REQUIRE(vars.contains({2, 0}) == true);
        REQUIRE(vars.contains({2, 1}) == true);
        REQUIRE(vars.contains({2, 2}) == true);

        run_test_block(solver, lp_problem, vars);

        auto cell00 = field->cell_at({0, 0});
        REQUIRE(cell00.is_marked_as_landmine());
        REQUIRE(cell00.is_uncovered() == false);

        auto cell10 = field->cell_at({1, 0});
        REQUIRE(cell10.is_marked_as_landmine());
        REQUIRE(cell10.is_uncovered() == false);

        auto cell20 = field->cell_at({2, 0});
        REQUIRE(cell20.is_uncovered() == true);
        REQUIRE(field->nearby_landmines_count({2, 0}) == 1);

        auto cell21 = field->cell_at({2, 1});
        REQUIRE(cell21.is_uncovered() == true);
        REQUIRE(field->nearby_landmines_count({2, 1}) == 1);

        auto cell22 = field->cell_at({2, 2});
        REQUIRE(cell22.is_uncovered() == true);
        REQUIRE(field->nearby_landmines_count({2, 2}) == 0);
    }
}
