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

    std::vector<size_t> expired;
    for (size_t i = 0; i < _timers.size(); ++i)
    {
        if (now >= _timers[i].NextFireTime)
            expired.push_back(i);
    }

    std::vector<uint64_t> toRemove;
    for (auto it = expired.rbegin(); it != expired.rend(); ++it)
    {
        size_t idx = *it;
        if (idx >= _timers.size())
            continue;

        auto& timer = _timers[idx];
        if (timer.Callback)
            timer.Callback();

        if (timer.Interval > 0)
        {
            timer.NextFireTime = now + timer.Interval;
        }
        else
        {
            toRemove.push_back(timer.Id);
        }
    }

    for (uint64_t id : toRemove)
    {
        _timers.erase(std::remove_if(_timers.begin(), _timers.end(), [id](const Timer& t) { return t.Id == id; }),
                      _timers.end());
    }
}

}  // namespace CS2Kit::Core
