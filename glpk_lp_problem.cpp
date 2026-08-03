#include "glpk_lp_problem.hpp"
#include "logger.hpp"
#include <cstring>
#include <filesystem>

namespace lp {

void matrix::reset()
{
    columns_.clear();
    rows_.clear();
    values_.clear();
    add(0, 0, 0); // dummy row, used due to the way glp_load_matrix works
}

const char* problem::kErrorMessages[] =
  { //
    "OK",
    "EBADB: 0x01 invalid basis",
    "ESING: 0x02 singular matrix",
    "ECOND: 0x03 ill-conditioned matrix",
    "EBOUND: 0x04 invalid bounds",
    "EFAIL: 0x05 solver failed",
    "EOBJLL: 0x06 objective lower limit reached",
    "EOBJUL: 0x07 objective upper limit reached",
    "EITLIM: 0x08 iteration limit exceeded",
    "ETMLIM: 0x09 time limit exceeded",
    "ENOPFS: 0x0A no primal feasible solution",
    "ENODFS: 0x0B no dual feasible solution",
    "EROOT: 0x0C root LP optimum not provided",
    "ESTOP: 0x0D search terminated by application",
    "EMIPGAP: 0x0E relative mip gap tolerance reached",
    "ENOFEAS: 0x0F no primal/dual feasible solution",
    "ENOCVG: 0x10 no convergence",
    "EINSTAB: 0x11 numerical instability",
    "EDATA: 0x12 invalid data",
    "ERANGE: 0x13 result out of range",
    nullptr};

problem::problem()
{
    glp_ = glp_create_prob();
    glp_init_smcp(&glp_opt_);
    set_verbose(GLP_MSG_OFF);
}

problem::~problem()
{
    glp_delete_prob(glp_);
}

void problem::set_matrix(const matrix& m)
{
    glp_load_matrix(
      glp_,
      m.get_columns().size() - 1,
      m.get_rows().data(),
      m.get_columns().data(),
      m.get_values().data());
}

void problem::dump_solution()
{
    xlog << std::fixed << "objective: " << get_objective_value();
    for (int i = 1; i <= get_num_columns(); ++i)
    {
        xlog << "  x[" << i << "]=" << get_column_primal(i);
    }
}

void problem::dump()
{
    auto base_path = std::filesystem::temp_directory_path() / "qed-XXXXXX";
    if (!mkdtemp(base_path.string().data()))
    {
        errlog << "Could not create temporary file: " << strerror(errno) << "\n";
        return;
    }

    // CPLEX LP format, human readable
    auto problem_lp_file_name = (base_path / "problem.lp").string();
    glp_write_lp(glp_, nullptr, problem_lp_file_name.data());

    // GLPK format
    auto problem_glpk_file_name = (base_path / "problem.glpk").string();
    glp_write_prob(glp_, 0, problem_glpk_file_name.data());

    // formatted primal/dual solution
    auto solution_file_name = (base_path / "solution.txt").string();
    glp_print_sol(glp_, solution_file_name.data());

    // sensitivity analysis
    auto sensitivity_analysis_file_name = (base_path / "sensitivity_analysis.txt").string();
    glp_print_ranges(glp_, 0, nullptr, 0, sensitivity_analysis_file_name.data());

    errlog << "Dumped data:\n"
           << problem_lp_file_name << "\n"
           << problem_glpk_file_name << "\n"
           << solution_file_name << "\n"
           << sensitivity_analysis_file_name << "\n";
}

std::expected<void, int> problem::presolve()
{
    glp_opt_.presolve = GLP_ON;
    auto result = solve();
    glp_opt_.presolve = GLP_OFF;
    return result;
}

} // namespace lp
