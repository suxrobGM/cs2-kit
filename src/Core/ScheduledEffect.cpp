#include <CS2Kit/Core/ScheduledEffect.hpp>
#include <CS2Kit/Core/Services.hpp>

#include <utility>

namespace CS2Kit::Core
{

struct ScheduledEffect::State
{
    uint64_t tickTimer = 0;
    uint64_t stopTimer = 0;
    std::function<void()> onStop;
    bool stopped = false;

    void Stop()
    {
        if (stopped)
            return;
        stopped = true;

        if (auto* kit = KitOrNull())
        {
            if (tickTimer)
                kit->Scheduler.Cancel(tickTimer);
            if (stopTimer)
                kit->Scheduler.Cancel(stopTimer);
        }
        tickTimer = 0;
        stopTimer = 0;

        if (onStop)
        {
            auto cb = std::move(onStop);
            onStop = nullptr;
            cb();
        }
    }
};

ScheduledEffect::ScheduledEffect(int64_t tickIntervalMs, int64_t durationMs, std::function<void()> onTick,
                                 std::function<void()> onStop)
    : _state(std::make_shared<State>())
{
    _state->onStop = std::move(onStop);

    auto& sched = Kit().Scheduler;
    if (onTick && tickIntervalMs > 0)
        _state->tickTimer = sched.Repeat(tickIntervalMs, std::move(onTick));

    if (durationMs > 0)
    {
        std::weak_ptr<State> weak = _state;
        _state->stopTimer = sched.Delay(durationMs, [weak]() {
            if (auto s = weak.lock())
                s->Stop();
        });
    }
}

ScheduledEffect::~ScheduledEffect()
{
    if (_state)
        _state->Stop();
}

ScheduledEffect& ScheduledEffect::operator=(ScheduledEffect&& other) noexcept
{
    if (this != &other)
    {
        if (_state)
            _state->Stop();
        _state = std::move(other._state);
    }
    return *this;
}

void ScheduledEffect::Stop()
{
    if (_state)
        _state->Stop();
}

bool ScheduledEffect::Active() const
{
    return _state && !_state->stopped;
}

}  // namespace CS2Kit::Core
