#include <CS2Kit/Core/ScheduledEffect.hpp>
#include <CS2Kit/Core/Scheduler.hpp>
#include <utility>

namespace CS2Kit::Core
{

struct ScheduledEffect::State
{
    Scheduler* scheduler = nullptr;
    uint64_t tickTimer = 0;
    uint64_t stopTimer = 0;
    std::function<void()> onStop;
    bool stopped = false;

    void Stop()
    {
        if (stopped)
            return;
        stopped = true;

        if (scheduler)
        {
            if (tickTimer)
                scheduler->Cancel(tickTimer);
            if (stopTimer)
                scheduler->Cancel(stopTimer);
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

ScheduledEffect::ScheduledEffect(Scheduler& scheduler, int64_t tickIntervalMs, int64_t durationMs,
                                 std::function<void()> onTick, std::function<void()> onStop)
    : _state(std::make_shared<State>())
{
    _state->scheduler = &scheduler;
    _state->onStop = std::move(onStop);

    if (onTick && tickIntervalMs > 0)
        _state->tickTimer = scheduler.Repeat(tickIntervalMs, std::move(onTick));

    if (durationMs > 0)
    {
        std::weak_ptr<State> weak = _state;
        _state->stopTimer = scheduler.Delay(durationMs, [weak]() {
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
