#pragma once

#include "board.hpp"
#include "game_board_widget.hpp"

#include <condition_variable>
#include <deque>
#include <functional>
#include <thread>

namespace qed
{

class Solver {
    static constexpr const size_t kUpdateRange = 1;
    enum class RunState : uint8_t {
	kNew,
	kRunning,
	kSuspending,
	kSuspended,
	kExit,
    };
    
public:
    enum FeedbackState : uint8_t {
	kSolved,
	kSuspended,
	kGameLost
    };

    using ResultHandler = std::function<void(FeedbackState, FieldPosition, size_t)>;

    explicit Solver(GameBoardPtr board) : board_{board} {}
    virtual ~Solver();
    
    bool isRunning() const;
    void startAsync();
    void suspend();
    void resume();
    void stop();
    void addPoi(FieldPosition);
    void setResultHandler(ResultHandler h) { resultHandler_ = h; }
    
protected:
    virtual bool doPoi(FieldPosition) = 0;

    struct NeighborhoodInfo {
        uint8_t landmines_count{}; // number of landmines left around current cell
        uint8_t nr{};       // size of "coveredUnmarkedFieldPositions" array
        std::array<FieldPosition, 8> coveredUnmarkedFieldPositions;
    };
    NeighborhoodInfo getNeighborhoodInfo(FieldPosition) const;

    GameBoardPtr board_;
    ResultHandler resultHandler_;
    
private:
    bool okToRun();
    void asyncSolver();
    
    std::atomic<RunState> state_{RunState::kNew};

    std::mutex queue_mtx_;     // used to protect queue access
    std::deque<FieldPosition> poi_; // a list of cells of interest

    std::thread thread_;
    std::mutex mtx_;
    std::condition_variable cond_;
};

} // namespace qed
