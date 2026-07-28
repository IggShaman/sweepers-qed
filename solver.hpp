#pragma once

#include "board.hpp"

#include <condition_variable>
#include <deque>
#include <functional>
#include <thread>

namespace qed
{

class Solver
{
    enum class RunState : uint8_t
    {
        kNew,
        kRunning,
        kSuspended,
        kExit,
    };

public:
    enum SolverState : uint8_t
    {
        kSolved,
        kSuspended,
        kGameLost
    };

    using result_handler_type = std::function<void(SolverState, FieldPosition, std::string)>;

    explicit Solver(GameBoardPtr board) : board_{board} {}
    virtual ~Solver();

    void startAsync();
    void suspend();
    void resume();
    void stop();
    void addPoi(FieldPosition);
    void setResultHandler(result_handler_type handler) { result_handler_ = handler; }

protected:
    virtual bool doPoi(FieldPosition) = 0;

    struct NeighborhoodInfo
    {
        uint8_t landmines_count{}; // number of landmines left around current cell
        uint8_t covered_unmarked_field_positions_count{};
        std::array<FieldPosition, 8> covered_unmarked_field_positions;
    };
    NeighborhoodInfo getNeighborhoodInfo(FieldPosition) const;

    GameBoardPtr board_;
    result_handler_type result_handler_;

private:
    bool ok_to_run();
    void async_solver_runner();

    std::atomic<RunState> state_{RunState::kNew};

    std::mutex queue_mutex_;        // used to protect queue access
    std::deque<FieldPosition> poi_; // cells of interest

    std::thread thread_;
    std::mutex runner_mutex_;
    std::condition_variable cond_;
};

} // namespace qed
