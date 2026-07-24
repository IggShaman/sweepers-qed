#pragma once

#include "solver.hpp"

namespace lp { class problem; }

namespace landmine {

class GlpkSolver : public Solver {
public:
    static constexpr float kEpsilon = 1e-3;
    static constexpr size_t kRange = 7;
    using Solver::Solver;
    
private:
    // maps location to variable id in an LP
    using VariablesMapType = std::unordered_map<Location, int>;

    void prepare(lp::problem*, landmine::Location, VariablesMapType&);
    bool doPoi(landmine::Location) override;
};

} // namespace landmine
