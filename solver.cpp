#include "solver.hpp"
#include "logger.hpp"

#include <thread>

namespace qed
{

Solver::~Solver()
{
    stop();

    if (thread_.joinable())
    {
        thread_.join();
    }
}

void Solver::startAsync()
{
    I_ASSERT(
      state_ == RunState::kNew,
      EX_LOG("state==" << static_cast<int>(state_.load()) << " != kNew"));

    I_ASSERT(!thread_.joinable(), EX_LOG("thread is joinable"));

    state_ = RunState::kSuspended;
    thread_ = std::thread(&Solver::async_solver_runner, this);
}

bool Solver::ok_to_run()
{
    std::unique_lock lock{runner_mutex_};

    cond_.wait(
      lock,
      [this] { return state_ != RunState::kSuspended and state_ != RunState::kNew; });

    return state_ == RunState::kRunning;
}

void Solver::suspend()
{
    I_ASSERT(state_ != RunState::kExit, EX_LOG("state == kExit"));
    {
        std::lock_guard<std::mutex> lock(runner_mutex_);
        state_ = RunState::kSuspended;
    }
    cond_.notify_one();
}

void Solver::resume()
{
    I_ASSERT(state_ != RunState::kExit, EX_LOG("state == kExit"));
    {
        std::lock_guard<std::mutex> lock(runner_mutex_);
        state_ = RunState::kRunning;
    }
    cond_.notify_one();
}

void Solver::stop()
{
    {
        std::lock_guard<std::mutex> lock(runner_mutex_);
        state_ = RunState::kExit;
    }
    cond_.notify_one();
}

void Solver::addPoi(FieldPosition position)
{
    std::lock_guard<std::mutex> lock{queue_mutex_};
    poi_.push_back(position);
}

Solver::NeighborhoodInfo Solver::getNeighborhoodInfo(FieldPosition position) const
{
    NeighborhoodInfo neighborhood;

    auto cell_info = board_->at(position);
    switch (cell_info)
    {
    case GameBoard::CellInfo::Exploded:
    case GameBoard::CellInfo::MarkedLandmine:
    case GameBoard::CellInfo::Unknown:
        I_FAIL(
          "internal error: cell " << position << " is of type " << static_cast<int>(cell_info)
                                  << ": not a free open one");
        break;

    case GameBoard::CellInfo::N0:
    case GameBoard::CellInfo::N1:
    case GameBoard::CellInfo::N2:
    case GameBoard::CellInfo::N3:
    case GameBoard::CellInfo::N4:
    case GameBoard::CellInfo::N5:
    case GameBoard::CellInfo::N6:
    case GameBoard::CellInfo::N7:
    case GameBoard::CellInfo::N8:
        neighborhood.landmines_count = static_cast<index_type>(cell_info);
        break;
    };

    {
        auto it = board_->neighborhood(position);
        while (it)
        {
            switch (it.at())
            {
            case GameBoard::CellInfo::MarkedLandmine:
                if (neighborhood.landmines_count > 0)
                {
                    --neighborhood.landmines_count;
                }
                break;

            case GameBoard::CellInfo::Unknown:
                neighborhood.covered_unmarked_field_positions
                  [neighborhood.covered_unmarked_field_positions_count++] = *it;
                break;

            default:
                break;
            }

            ++it;
        }
    }

    return neighborhood;
}

void Solver::async_solver_runner()
{
    while (ok_to_run())
    {
        FieldPosition poi;

        {
            std::unique_lock<std::mutex> lock{queue_mutex_};
            if (poi_.empty())
            {
                state_ = RunState::kSuspended;
                lock.unlock();
                result_handler_(SolverState::kSuspended, {}, {});
                continue;
            }

            poi = poi_.front();
            poi_.pop_front();
        }

        if (!doPoi(poi))
        {
            std::unique_lock<std::mutex> lock{queue_mutex_};
            state_ = RunState::kExit;
            return;
        }

        result_handler_(SolverState::kSolved, poi, {});
    }
}
} // namespace qed
