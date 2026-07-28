#include "glpk_solver.hpp"
#include "glpk_lp_problem.hpp"

#include "logger.hpp"

namespace qed
{

struct lp_row_info {
    lp_row_info(uint8_t v, std::string n) : fixed_value{v}, name{n} {}

    uint8_t fixed_value;
    std::string name;
};

bool GlpkSolver::doPoi(qed::FieldPosition poi)
{
    if (board_->is_uncovered(poi))
    {
        auto pois = getNeighborhoodInfo(poi);
        if (!pois.covered_unmarked_field_positions_count)
        {
            return true;
        }
    }

    auto lp = std::make_unique<lp::problem>();

    // a set of locations LP is looking at; maps coord to LP's column variable number
    VariablesMapType vars;
    prepare(lp.get(), poi, vars);
    if (vars.empty())
    {
        return true;
    }

    for (auto& v : vars)
    {
        lp->set_objective_coefficient(v.second, 1);
        lp->set_maximize();
        lp->solve();

        auto obj = lp->get_objective_value();
        if (obj <= 1 - kEpsilon)
        {
            // can't have a mine here
            if (board_->field()->is_landmine(v.first))
            {
                std::ostringstream oss;
                oss << "Game is lost at " << v.first << ": shold've been empty, has a mine"
                    << "\nobjective: " << obj << "\npoi=" << poi << "\nLP: " << lp->dump();
                // board_->dump_region(poi, 3);
                result_handler_(SolverState::kGameLost, {}, oss.str());
                return false;
            }

            board_->uncovered_safe(v.first, board_->field()->nearby_landmines_count(v.first));
            lp->set_column_fixed_bound(v.second, 0);
            addPoi(v.first);
        }
        else
        {
            lp->set_minimize();
            lp->solve();
            auto obj = lp->get_objective_value();
            if (obj >= kEpsilon) // must have a mine here
            {
                if (!board_->field()->is_landmine(v.first))
                {
                    std::ostringstream oss;
                    oss << "Calculated " << v.first << " to contain a mine, but it doesn't"
                        << "\nobjective: " << obj << "\npoi=" << poi << "\nLP: " << lp->dump();
                    // board_->dump_region(poi, 3);
                    result_handler_(SolverState::kGameLost, {}, oss.str());
                    return false;
                }

                board_->mark_mine(v.first, true);
                lp->set_column_fixed_bound(v.second, 1);
                addPoi(v.first);
            }
        }

        lp->set_objective_coefficient(v.second, 0);
    }

    return true;
}

void GlpkSolver::prepare(lp::problem* lp, FieldPosition poi, VariablesMapType& vars) {
    std::ostringstream oss;

    int vars_count{};

    //
    // setup initial LP
    //
    std::vector<lp_row_info> rows; // used to set row names and constraints
    lp::matrix m;

    for (index_type row = poi.row > kRange ? poi.row - kRange : 0;
         row <= std::min(board_->rows() - 1, poi.row + kRange);
         ++row)
    {
        for (index_type col = poi.column > kRange ? poi.column - kRange : 0;
             col <= std::min(board_->columns() - 1, poi.column + kRange);
             ++col)
        {
            FieldPosition position{row, col};
            auto cell_info = board_->at(position);
            if (static_cast<int>(cell_info) < 0)
            {
                continue;
            }

            auto pois = getNeighborhoodInfo(position);
            if (!pois.covered_unmarked_field_positions_count)
            {
                continue;
            }

            oss.str("");
            oss << 'n' << position;
            rows.push_back({pois.landmines_count, oss.str()});

            for (uint8_t idx = 0; idx < pois.covered_unmarked_field_positions_count; ++idx)
            {
                // find/add column variable for an uncovered cell
                auto iv = vars.insert({pois.covered_unmarked_field_positions[idx], vars_count + 1});
                if (iv.second)
                    ++vars_count;

                // set coefficient to 1
                m.add(rows.size(), iv.first->second, 1);
            }
        }
    }

    if (!vars_count)
        return;

    //
    // add variables, populate constraints
    //

    // add rows
    lp->add_row_variables(rows.size());
    for (index_type row = 0; row < rows.size(); ++row)
    {
        lp->set_row_name(row + 1, rows[row].name.data());
        lp->set_row_fixed_bound(row + 1, rows[row].fixed_value);
    }

    // add columns
    lp->add_column_variables(vars.size());
    for (auto v : vars)
    {
        oss.str("");
        oss << 'u' << v.first;
        lp->set_column_name(v.second, oss.str().data());
        lp->set_column_bounded(v.second, 0, 1);
    }

    lp->set_matrix(m);

    if (!lp->presolve())
    {
        errlog << "ERROR: could not presolve: " << lp->last_errmsg() << "\npoi=" << poi
               << "\nLP: " << lp->dump() << "\n";
        board_->dump_region(poi, kRange);
        std::abort();
    }
}

} // namespace qed
