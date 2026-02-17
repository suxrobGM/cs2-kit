#pragma once

#include <CS2Kit/Core/Singleton.hpp>

#include <cstdint>
#include <functional>
#include <vector>

namespace CS2Kit::Core
{

/**
 * Tick-based task scheduler for one-shot delays and repeating timers.
 * Driven by OnGameFrame() which is called every server tick.
 * All callbacks execute on the game thread (no synchronization needed).
 */
class Scheduler : public Singleton<Scheduler>
{
public:
    explicit Scheduler(Token) {}

    uint64_t Delay(int64_t delayMs, std::function<void()> callback);
    uint64_t Repeat(int64_t intervalMs, std::function<void()> callback);
    uint64_t DelayAndRepeat(int64_t delayMs, int64_t intervalMs, std::function<void()> callback);
    uint64_t NextTick(std::function<void()> callback);
    void Cancel(uint64_t id);
    void CancelAll();
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
