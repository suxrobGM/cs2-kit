#include <CS2Kit/Core/EffectManager.hpp>
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
    auto it = _effects[slot].find(effectId);
    // A self-expired effect (DurationMs elapsed) leaves its entry behind until reclaimed; its
    // ScheduledEffect reports inactive, so treat that as not-active without touching the map.
    return it != _effects[slot].end() && it->second.Fx.Active();
}

void EffectManager::Apply(int slot, int effectId, EffectSpec spec)
{
    if (!ValidSlot(slot))
        return;

    Cancel(slot, effectId);  // re-apply semantics: replace any active instance

    _effects[slot].insert_or_assign(
        effectId, ActiveEffect{.RoundScoped = spec.RoundScoped,
                               .Fx = ScheduledEffect(_scheduler, spec.TickIntervalMs, spec.DurationMs,
                                                     std::move(spec.OnTick), std::move(spec.OnStop))});
}

void EffectManager::Cancel(int slot, int effectId)
{
    if (!ValidSlot(slot))
        return;
    auto it = _effects[slot].find(effectId);
    if (it == _effects[slot].end())
        return;

    // Detach before stopping so a re-entrant Apply sees a clean slot. Stop() runs onStop once
    // (a no-op if the effect already self-expired).
    ActiveEffect entry = std::move(it->second);
    _effects[slot].erase(it);
    entry.Fx.Stop();
}

void EffectManager::CancelWhere(int slot, const std::function<bool(const ActiveEffect&)>& keep)
{
    std::vector<int> ids;
    ids.reserve(_effects[slot].size());
    for (const auto& [id, entry] : _effects[slot])
        if (keep(entry))
            ids.push_back(id);
    for (int id : ids)
        Cancel(slot, id);
}

void EffectManager::CancelAllForSlot(int slot)
{
    if (!ValidSlot(slot))
        return;
    CancelWhere(slot, [](const ActiveEffect&) { return true; });
}

void EffectManager::CancelRoundScoped()
{
    for (int slot = 0; slot < MaxSlots; ++slot)
        CancelWhere(slot, [](const ActiveEffect& e) { return e.RoundScoped; });
}

void EffectManager::CancelAll()
{
    for (int slot = 0; slot < MaxSlots; ++slot)
        CancelAllForSlot(slot);
}

}  // namespace CS2Kit::Core
