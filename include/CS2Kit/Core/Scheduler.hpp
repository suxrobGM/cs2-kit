#pragma once

#include <CS2Kit/Core/Singleton.hpp>
#include <cstdint>
#include <functional>
#include <vector>

namespace CS2Kit::Core
{

/**
 * @brief Tick-based task scheduler for one-shot delays and repeating timers.
 * Driven by `OnGameFrame()` (called every server tick from the plugin's GameFrame hook).
 * All callbacks execute on the game thread; no synchronization required.
 */
class Scheduler : public Singleton<Scheduler>
{
public:
    explicit Scheduler(Token) {}

    /** Run `callback` once after `delayMs` milliseconds. Returns a cancellation handle. */
    uint64_t Delay(int64_t delayMs, std::function<void()> callback);

    /** Run `callback` every `intervalMs` milliseconds. Returns a cancellation handle. */
    uint64_t Repeat(int64_t intervalMs, std::function<void()> callback);

    /** First fire after `delayMs`, then repeat every `intervalMs`. Returns a cancellation handle. */
    uint64_t DelayAndRepeat(int64_t delayMs, int64_t intervalMs, std::function<void()> callback);

    /** Run `callback` on the very next game frame. */
    uint64_t NextTick(std::function<void()> callback);

    /** Cancel a timer by handle. Safe to call with an unknown id. */
    void Cancel(uint64_t id);

    /** Cancel all pending timers. */
    void CancelAll();

    /** Drive the scheduler (call from your `GameFrame` hook or via `CS2Kit::OnGameFrame()`). */
    void OnGameFrame();

private:
    struct Timer
    {
        uint64_t Id;
        int64_t NextFireTime;
        int64_t Interval;
        std::function<void()> Callback;
    };

    int64_t GetCurrentTimeMs() const;

    std::vector<Timer> _timers;
    uint64_t _nextId = 1;
};

}  // namespace CS2Kit::Core
