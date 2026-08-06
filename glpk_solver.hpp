#pragma once

#include "solver.hpp"

namespace lp { class problem; }

namespace qed
{
class GlpkSolver : public Solver
{
    friend struct glpk_solver_access;

public:
    // maps field_position to variable id in an LP
    using variables_map_type = std::unordered_map<field_position, int>;

    static constexpr float kEpsilon = 1e-5;
    static constexpr index_type kRange = 7;
    using Solver::Solver;

    ~GlpkSolver() override;

    std::expected<solver_step_stats, std::string> take_stats_sample() override;

private:
    bool doPoi(field_position) override;
    void prepare_block(lp::problem&, field_position, variables_map_type&);
    bool test_block(lp::problem&, variables_map_type&);
};

} // namespace qed
