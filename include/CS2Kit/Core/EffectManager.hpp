#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace CS2Kit::Core
{

class Scheduler;

/**
 * @brief Per-target effect bookkeeping. Owns scheduler handles and a CancelFn that
 * each effect populates to undo its own state (restore render mode, kill timers, etc.).
 */
struct ActiveEffect
{
    bool RoundScoped = false;
    uint64_t TimerHandle = 0;
    uint64_t DurationHandle = 0; /**< Auto-expire timer, owned here and cancelled with the effect. */
    std::function<void()> CancelFn;
};

/** What an effect's enable step hands back to @ref EffectManager::Toggle to register it. */
struct EffectSetup
{
    uint64_t TimerHandle = 0;       /**< Scheduler handle owned by the effect, or 0. */
    std::function<void()> CancelFn; /**< Undo the effect's state (restore render, team, etc.). */
    bool RoundScoped = false;       /**< Auto-cancel via @ref EffectManager::CancelRoundScoped. */
    int DurationMs = 0; /**< >0 auto-cancels the effect after this long; the timer is owned by EffectManager. */
};

/**
 * @brief Per-slot registry of toggleable/timed player effects, keyed by a plugin-defined
 * integer id (cast your effect enum). Owns the auto-expire timers and re-toggle semantics.
 *
 * Deliberately plugin-owned rather than a kit service: cancel closures touch pawns and
 * timers, so the owning plugin must control when CancelAll runs relative to engine teardown.
 */
class EffectManager
{
public:
    static constexpr int MaxSlots = 64;

    explicit EffectManager(Scheduler& scheduler) : _scheduler(scheduler) {}

    bool IsActive(int slot, int effectId) const;

    /**
     * @brief Register a new effect for `slot`. If an effect of the same id is already
     * active, it is cancelled first (re-toggle semantics).
     *
     * @param timerHandle  Scheduler handle owned by the effect. Pass 0 for state-only effects.
     * @param cancelFn     Called when the effect is cancelled for any reason. Should restore
     *                     pawn state and Cancel(timerHandle) itself if applicable.
     * @param roundScoped  True for effects that should auto-cancel on round end.
     */
    void Apply(int slot, int effectId, uint64_t timerHandle, std::function<void()> cancelFn, bool roundScoped = false,
               int durationMs = 0);

    /**
     * @brief Toggle an effect. If it is active, cancel it and return false. Otherwise run
     * @p enable (which sets up the effect and returns its @ref EffectSetup), register it, and
     * return true. Lets callers collapse the IsActive/Cancel/Apply dance to one call and pick
     * the broadcast line from the bool: `Broadcast(ctx, on ? "...On" : "...Off")`.
     */
    bool Toggle(int slot, int effectId, const std::function<EffectSetup()>& enable);

    void Cancel(int slot, int effectId);
    void CancelAllForSlot(int slot);
    /** Cancel every active effect registered with `roundScoped == true`, on every slot. */
    void CancelRoundScoped();
    void CancelAll();

private:
    Scheduler& _scheduler;
    // Slot-indexed array of small maps: only a handful of effects run per player at once.
    std::array<std::unordered_map<int, ActiveEffect>, MaxSlots> _effects{};
};

}  // namespace CS2Kit::Core
