#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace CS2Kit::Core
{

/**
 * @brief A repeating routine with an optional fixed lifetime, driven by the Scheduler.
 *
 * `onTick` runs every `tickIntervalMs`. If `durationMs > 0` the effect auto-stops after that
 * long. `onStop` runs exactly once when the effect ends for any reason - duration elapsed,
 * Stop(), move-assignment, or destruction - making it the single place to undo whatever
 * `onTick` applied. Move-only; the underlying timers are cancelled when the last owner goes away.
 *
 * @code
 * effect = ScheduledEffect(200, 15000,
 *     [slot]{ CycleColor(slot); },        // every 200ms for 15s
 *     [slot]{ RestoreColor(slot); });     // once, when it ends
 * @endcode
 */
class ScheduledEffect
{
public:
    ScheduledEffect() = default;
    ScheduledEffect(int64_t tickIntervalMs, int64_t durationMs, std::function<void()> onTick,
                    std::function<void()> onStop);
    ~ScheduledEffect();

    ScheduledEffect(ScheduledEffect&&) noexcept = default;
    ScheduledEffect& operator=(ScheduledEffect&&) noexcept;
    ScheduledEffect(const ScheduledEffect&) = delete;
    ScheduledEffect& operator=(const ScheduledEffect&) = delete;

    /** Stop ticking and run `onStop` (once). Safe to call repeatedly. */
    void Stop();

    /** True while the effect is still running (not yet stopped). */
    bool Active() const;

private:
    struct State;
    std::shared_ptr<State> _state;
};

}  // namespace CS2Kit::Core
