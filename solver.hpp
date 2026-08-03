#pragma once

#include "byte_field.hpp"

#include <condition_variable>
#include <deque>
#include <functional>
#include <thread>

namespace qed
{
using byte_field_ptr = std::shared_ptr<byte_field>;

class Solver
{
    friend struct solver_access;

    enum class RunState
    {
        kNew,
        kRunning,
        kSuspended,
        kExit,
    };

public:
    enum class SolverState
    {
        kSolved,
        kSuspended,
        kGameLost
    };

    using result_handler_type = std::function<void(SolverState, field_position, std::string)>;

    explicit Solver(byte_field_ptr field) : field_{field} {}
    virtual ~Solver();

    void startAsync();
    void suspend();
    void resume();
    void stop();
    void addPoi(field_position);
    void setResultHandler(result_handler_type handler) { result_handler_ = handler; }
    // Used by e.g. benchmarks and unit tests; suspends until the solver
    // itself ends up in a "suspended" state.
    // Don't forget to run the solver first.
    void wait_for_completion();

protected:
    virtual bool doPoi(field_position) = 0;

    byte_field_ptr field_;
    result_handler_type result_handler_;

private:
    bool ok_to_run();
    void async_solver_runner();

    std::atomic<RunState> state_{RunState::kNew};

    std::mutex queue_mutex_;         // used to protect queue access
    std::deque<field_position> poi_; // cells of interest

    std::thread thread_;
    std::mutex runner_mutex_;
    std::condition_variable cond_;
};

} // namespace qed
