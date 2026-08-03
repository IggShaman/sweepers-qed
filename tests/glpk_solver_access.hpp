#include "field_position.hpp"
#include "glpk_solver.hpp"
#include "solver_access.hpp"

namespace qed
{
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
} // namespace qed
