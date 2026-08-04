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

void Solver::set_byte_field(byte_field_ptr field)
{
    if (field_)
    {
        errlog << "Field already set\n";
        abort();
    }
    field_ = field;
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

void Solver::addPoi(field_position position)
{
    std::lock_guard<std::mutex> lock{queue_mutex_};
    poi_.push_back(position);
}

void Solver::async_solver_runner()
{
    while (ok_to_run())
    {
        field_position poi;

        {
            std::unique_lock<std::mutex> lock{queue_mutex_};
            if (poi_.empty())
            {
                lock.unlock();

                {
                    std::unique_lock lock{runner_mutex_};
                    state_ = RunState::kSuspended;
                    cond_.notify_one();
                }

                if (result_handler_)
                {
                    result_handler_(SolverState::kSuspended, {}, {});
                }
                continue;
            }

            poi = poi_.front();
            poi_.pop_front();
        }

        if (!doPoi(poi))
        {
            std::unique_lock<std::mutex> lock{queue_mutex_};
            state_ = RunState::kExit;
            cond_.notify_one();
            return;
        }

        if (result_handler_)
        {
            result_handler_(SolverState::kSolved, poi, {});
        }
    }
}

void Solver::wait_for_completion()
{
    std::unique_lock lock{runner_mutex_};

    if (state_ != RunState::kRunning and state_ != RunState::kSuspended)
    {
        errlog << "Solver is not running or suspended!\n";
        abort();
    }

    cond_.wait(lock, [this] { return state_ == RunState::kSuspended; });
}
} // namespace qed
