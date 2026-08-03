#include "glpk_solver.hpp"
#include "glpk_lp_problem.hpp"

#include "logger.hpp"

namespace qed
{
struct lp_constraint_desc
{
    lp_constraint_desc(int _fixed_value, std::string _name) : fixed_value{_fixed_value}, name{_name}
    {
    }

    int fixed_value;
    std::string name;
};

GlpkSolver::~GlpkSolver()
{
    // NOTE: not strictly necessary.
    // glp_free_env();
}

bool GlpkSolver::doPoi(field_position poi)
{
    auto poi_cell = field_->cell_at(poi);

    lp::problem lp_problem;
    variables_map_type vars;

    prepare_block(lp_problem, poi, vars);
    return test_block(lp_problem, vars);
}

bool GlpkSolver::test_block(lp::problem& lp_problem, variables_map_type& vars)
{
    if (vars.empty())
    {
        return true;
    }

    // TODO: go over variables in a radial manner around the center
    for (auto& v : vars)
    {
        lp_problem.set_objective_coefficient(v.second, 1);
        lp_problem.set_maximize();
        lp_problem.solve();

        auto obj = lp_problem.get_objective_value();

        if (obj <= 1 - kEpsilon) // can't have a mine here
        {
            auto cell = field_->cell_at(v.first);
            if (cell.is_landmine_groundtruth())
            {
                std::ostringstream oss;
                oss << "Game is lost at " << v.first << ": should've been empty, has a mine";
                if (result_handler_)
                {
                    result_handler_(SolverState::kGameLost, {}, oss.str());
                }
                else
                {
                    errlog << oss.str() << "\n";
                }
                return false;
            }

            cell.set_uncovered();
            lp_problem.set_column_fixed_bound(v.second, 0);
            if (result_handler_)
            {
                result_handler_(SolverState::kSolved, v.first, {});
            }
            addPoi(v.first);
        }
        else
        {
            lp_problem.set_minimize();
            lp_problem.solve();
            obj = lp_problem.get_objective_value();
            if (obj >= kEpsilon) // must have a mine here
            {
                auto cell = field_->cell_at(v.first);
                if (!cell.is_landmine_groundtruth())
                {
                    std::ostringstream oss;
                    oss << "Calculated " << v.first << " to contain a mine, but it doesn't";
                    if (result_handler_)
                    {
                        result_handler_(SolverState::kGameLost, {}, oss.str());
                    }
                    else
                    {
                        errlog << oss.str() << "\n";
                    }
                    return false;
                }

                cell.set_marked_as_landmine();
                lp_problem.set_column_fixed_bound(v.second, 1);
                if (result_handler_)
                {
                    result_handler_(SolverState::kSolved, v.first, {});
                }
                addPoi(v.first);
            }
        }

        // clear objective coefficient
        lp_problem.set_objective_coefficient(v.second, 0);
    }

    return true;
}

void GlpkSolver::prepare_block(
  lp::problem& lp_problem,
  field_position poi,
  variables_map_type& new_vars)
{
    std::ostringstream oss;

    int vars_count{};
    std::vector<lp_constraint_desc> constraints;

    auto get_nearby_info =
      [this](field_position center) -> std::tuple<uint8_t, std::array<field_position, 8>, uint8_t>
    {
        uint8_t marked_landmines_count{};
        std::array<field_position, 8> xes;
        uint8_t xes_count{};

        for (auto x_pos : neighborhood(center, field_->rows(), field_->columns(), 1))
        {
            if (x_pos == center)
            {
                continue;
            }

            auto cell = field_->cell_at(x_pos);
            if (cell.is_marked_as_landmine())
            {
                ++marked_landmines_count;
                continue;
            }

            if (!cell.is_uncovered())
            {
                xes[xes_count++] = x_pos;
            }
        }

        return {marked_landmines_count, xes, xes_count};
    };

    auto get_variable_index = [&new_vars, &vars_count](field_position pos) -> int
    {
        auto [it, inserted] = new_vars.try_emplace(pos);
        if (inserted)
        {
            it->second = ++vars_count;
        }
        return it->second;
    };

    lp::matrix m;

    int constraints_count{};

    for (auto position : neighborhood(poi, field_->rows(), field_->columns(), kRange - 1))
    {
        auto cell = field_->cell_at(position);
        if (!cell.is_uncovered())
        {
            continue;
        }

        auto gt_mines_nearby = field_->nearby_landmines_count(position);
        auto [marked_landmines_count, near_var_pos, near_var_pos_count] = get_nearby_info(position);
        if (near_var_pos_count == 0 or gt_mines_nearby < marked_landmines_count)
        {
            continue;
        }

        ++constraints_count;
        for (uint8_t idx = 0; idx < near_var_pos_count; ++idx)
        {
            auto var_id = get_variable_index(near_var_pos[idx]);
            m.add(constraints_count, var_id, 1);
        }

        oss.str("");
        oss << position.row;
        constraints.push_back({gt_mines_nearby - marked_landmines_count, oss.str()});
    }

    if (!vars_count)
    {
        return;
    }

    // add variables (as columns)
    lp_problem.add_column_variables(vars_count);
    for (auto v : new_vars)
    {
        oss.str("");
        oss << v.first;
        lp_problem.set_column_name(v.second, oss.str().data());
        lp_problem.set_column_bounded(v.second, 0, 1);
    }

    // add constraints (rows)
    lp_problem.add_row_variables(constraints_count);
    for (index_type idx = 0; idx < constraints_count; ++idx)
    {
        lp_problem.set_row_name(idx + 1, constraints[idx].name.data());
        lp_problem.set_row_fixed_bound(idx + 1, constraints[idx].fixed_value);
    }

    lp_problem.set_matrix(m);

    if (!lp_problem.presolve())
    {
        errlog << "ERROR: could not presolve: " << lp_problem.last_errmsg() << "\npoi=" << poi
               << "\n";
        lp_problem.dump();
        std::abort();
    }
}

} // namespace qed
