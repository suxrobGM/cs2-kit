#include <CS2Kit/Core/Scheduler.hpp>

#include <algorithm>
#include <chrono>

namespace CS2Kit::Core
{

int64_t Scheduler::GetCurrentTimeMs() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

uint64_t Scheduler::Delay(int64_t delayMs, std::function<void()> callback)
{
    uint64_t id = _nextId++;
    _timers.push_back({id, GetCurrentTimeMs() + delayMs, 0, std::move(callback)});
    return id;
}

uint64_t Scheduler::Repeat(int64_t intervalMs, std::function<void()> callback)
{
    uint64_t id = _nextId++;
    _timers.push_back({id, GetCurrentTimeMs() + intervalMs, intervalMs, std::move(callback)});
    return id;
}

uint64_t Scheduler::DelayAndRepeat(int64_t delayMs, int64_t intervalMs, std::function<void()> callback)
{
    uint64_t id = _nextId++;
    _timers.push_back({id, GetCurrentTimeMs() + delayMs, intervalMs, std::move(callback)});
    return id;
}

uint64_t Scheduler::NextTick(std::function<void()> callback)
{
    return Delay(0, std::move(callback));
}

void Scheduler::Cancel(uint64_t id)
{
    _timers.erase(std::remove_if(_timers.begin(), _timers.end(), [id](const Timer& t) { return t.Id == id; }),
                  _timers.end());
}

void Scheduler::CancelAll()
{
    _timers.clear();
}

void Scheduler::OnGameFrame()
{
    if (_timers.empty())
        return;

    int64_t now = GetCurrentTimeMs();

    // Snapshot the IDs to fire instead of indices: callbacks may Cancel() other timers
    // (or themselves) which erases from _timers and invalidates indices/references.
    std::vector<uint64_t> toFire;
    for (const auto& t : _timers)
    {
        if (now >= t.NextFireTime)
            toFire.push_back(t.Id);
    }

    for (uint64_t id : toFire)
    {
        // Re-find by ID — the timer may have been cancelled by an earlier callback in this batch.
        auto it = std::find_if(_timers.begin(), _timers.end(), [id](const Timer& t) { return t.Id == id; });
        if (it == _timers.end())
            continue;

        // Copy out before invoking — the callback can mutate _timers (including reallocating it).
        int64_t interval = it->Interval;
        auto callback = it->Callback;

        if (callback)
            callback();

        // Re-find again: the callback may have cancelled this timer.
        it = std::find_if(_timers.begin(), _timers.end(), [id](const Timer& t) { return t.Id == id; });
        if (it == _timers.end())
            continue;

        if (interval > 0)
            it->NextFireTime = now + interval;
        else
            _timers.erase(it);
    }
}

}  // namespace CS2Kit::Core
