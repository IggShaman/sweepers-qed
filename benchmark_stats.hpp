#pragma once

#include "field_config.hpp"
#include "solver_stats.hpp"

#include <chrono>
#include <expected>
#include <string>
#include <vector>

namespace qed
{
std::expected<void, std::string> log_build_info(const std::string& dsn);

std::expected<void, std::string>
log_solver_run_stats(const std::string& dsn, const std::vector<qed::solver_run_stats>&);

std::expected<void, std::string>
log_solver_step_stats(const std::string& dsn, const std::vector<qed::solver_step_stats>&);
} // namespace qed
