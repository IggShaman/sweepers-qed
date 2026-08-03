#include "solver.hpp"

namespace qed
{
struct solver_access
{
protected:
    static std::deque<field_position>& get_poi(Solver& s) { return s.poi_; }
};
} // namespace qed
