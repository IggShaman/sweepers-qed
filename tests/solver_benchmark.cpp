#include "byte_field_image_saver.hpp"
#include "field_utils.hpp"
#include "glpk_lp_problem.hpp"
#include "glpk_solver.hpp"
#include "glpk_solver_access.hpp"
#include "logger.hpp"
#include "minefield_generator.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>

namespace b
{
struct field_config
{
    qed::index_type rows;
    qed::index_type columns;
    std::size_t landmine_count;
    size_t seed;

    std::vector<qed::field_position> initial_pois;
    size_t final_uncovered_positions;
    size_t final_landmines_marked;
};

void find_empty_initial_pois(qed::byte_field* field, int find_count)
{
    int found{};

    for (auto j = static_cast<int>(field->rows() / 2) - 10;
         j < static_cast<int>(field->rows() / 2) + 10;
         ++j)
    {
        for (auto i = static_cast<int>(field->columns() / 2) - 10;
             i < static_cast<int>(field->columns() / 2) + 10;
             ++i)
        {
            auto pos = qed::field_position(j, i);
            auto cell = field->cell_at(pos);
            if (
              !cell.is_landmine_groundtruth() and field->nearby_landmines_count(pos) == 0 and
              ++found <= find_count)
            {
                errlog << "pos:" << pos << " cell:" << cell << "\n";
            }
        }
    }
}

struct scoped_timer
{
    auto tdiff() { return std::chrono::steady_clock::now() - t0; }

    std::chrono::steady_clock::time_point t0{std::chrono::steady_clock::now()};
};

struct glpk_benchmark
{
    bool run_glpk_solver(const field_config& field_config);
};

bool glpk_benchmark::run_glpk_solver(const field_config& field_config)
{
    auto field = std::make_shared<qed::byte_field>();
    field->reset(field_config.rows, field_config.columns);
    if (!qed::generate_minefield(field.get(), field_config.landmine_count, field_config.seed))
    {
        errlog << "field generation failed\n";
        return false;
    }
    if (qed::count_field_stats(field.get()).landmines != field_config.landmine_count)
    {
        errlog << "wrong landmine_count\n";
        return false;
    }

    // launch solver
    auto solver = std::make_shared<qed::GlpkSolver>(field);
    solver->startAsync();

    for (auto poi : field_config.initial_pois)
    {
        auto cell = field->cell_at(poi);
        if (cell.is_landmine_groundtruth())
        {
            errlog << "initial poi " << poi << " has a landmine\n";
            abort();
        }
        cell.set_uncovered();

        solver->addPoi(poi);
    }

    {
        b::scoped_timer timer;
        solver->resume();
        solver->wait_for_completion();
        xlog << "runtime:" << timer.tdiff() << "\n";
    }

    solver->stop();
    solver.reset();

    auto field_stats = count_field_stats(field.get());
    if (field_stats.landmines != field_config.landmine_count)
    {
        errlog << SHOW_(field_stats.landmines) << " != " << SHOW(field_config.landmine_count)
               << "\n";
        abort();
    }

    if (field_stats.uncovered != field_config.final_uncovered_positions)
    {
        errlog << SHOW_(field_stats.uncovered)
               << " != " << SHOW(field_config.final_uncovered_positions) << "\n";
        abort();
    }

    if (field_stats.marked_as_mine != field_config.final_landmines_marked)
    {
        errlog << SHOW_(field_stats.marked_as_mine)
               << " != " << SHOW(field_config.final_landmines_marked) << "\n";
    }

    qed::export_field_to_png(field.get(), "output.png");

    return true;
}
} // namespace b

int main()
{
    auto field_configs = std::initializer_list<b::field_config>{
      {512, 512, 52428, 0, std::initializer_list<qed::field_position>{{50, 60}}, 209281, 52181},
    };

    b::glpk_benchmark glpk_benchmark;
    for (const auto& config : field_configs)
    {
        if (!glpk_benchmark.run_glpk_solver(config))
        {
            errlog << "glpk_benchmark failed\n";
            abort();
        }
    }

    return 0;
}
