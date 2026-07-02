#include <CS2Kit/Core/EffectManager.hpp>
#include <CS2Kit/Core/Scheduler.hpp>
#include <utility>
#include <vector>

namespace CS2Kit::Core
{

namespace
{
bool ValidSlot(int slot)
{
    return slot >= 0 && slot < EffectManager::MaxSlots;
}
}  // namespace

bool EffectManager::IsActive(int slot, int effectId) const
{
    if (!ValidSlot(slot))
        return false;
    return _effects[slot].contains(effectId);
}

void EffectManager::Apply(int slot, int effectId, uint64_t timerHandle, std::function<void()> cancelFn,
                          bool roundScoped, int durationMs)
{
    if (!ValidSlot(slot))
        return;

    Cancel(slot, effectId);  // re-toggle semantics: replace any active instance

    ActiveEffect entry;
    entry.RoundScoped = roundScoped;
    entry.TimerHandle = timerHandle;
    entry.CancelFn = std::move(cancelFn);
    // Own the auto-expire timer here (not in the effect) so cancelling the effect always cancels it too --
    // a self-scheduled duration timer would survive an early cancel and later clobber a re-applied effect.
    entry.DurationHandle =
        durationMs > 0 ? _scheduler.Delay(durationMs, [this, slot, effectId]() { Cancel(slot, effectId); }) : 0;
    _effects[slot][effectId] = std::move(entry);
}

bool EffectManager::Toggle(int slot, int effectId, const std::function<EffectSetup()>& enable)
{
    if (!ValidSlot(slot))
        return false;
    if (IsActive(slot, effectId))
    {
        Cancel(slot, effectId);
        return false;
    }
    EffectSetup setup = enable();
    Apply(slot, effectId, setup.TimerHandle, std::move(setup.CancelFn), setup.RoundScoped, setup.DurationMs);
    return true;
}

void EffectManager::Cancel(int slot, int effectId)
{
    if (!ValidSlot(slot))
        return;
    auto it = _effects[slot].find(effectId);
    if (it == _effects[slot].end())
        return;

    // Detach before running CancelFn so a re-entrant Apply/Toggle sees a clean slot.
    ActiveEffect entry = std::move(it->second);
    _effects[slot].erase(it);

    if (entry.CancelFn)
        entry.CancelFn();
    if (entry.TimerHandle != 0)
        _scheduler.Cancel(entry.TimerHandle);
    if (entry.DurationHandle != 0)
        _scheduler.Cancel(entry.DurationHandle);
}

void EffectManager::CancelAllForSlot(int slot)
{
    if (!ValidSlot(slot))
        return;
    std::vector<int> ids;
    ids.reserve(_effects[slot].size());
    for (const auto& [id, entry] : _effects[slot])
        ids.push_back(id);
    for (int id : ids)
        Cancel(slot, id);
}

void EffectManager::CancelRoundScoped()
{
    for (int slot = 0; slot < MaxSlots; ++slot)
    {
        std::vector<int> ids;
        for (const auto& [id, entry] : _effects[slot])
        {
            if (entry.RoundScoped)
                ids.push_back(id);
        }
        for (int id : ids)
            Cancel(slot, id);
    }
}

void EffectManager::CancelAll()
{
    for (int slot = 0; slot < MaxSlots; ++slot)
        CancelAllForSlot(slot);
}

}  // namespace CS2Kit::Core
