#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Core/Slot.hpp>
#include <CS2Kit/Sdk/MovementHook.hpp>
#include <CS2Kit/Utils/Log.hpp>

#include <safetyhook.hpp>

using CS2Kit::Core::Engine;

namespace CS2Kit::Sdk
{
using namespace CS2Kit::Utils;

namespace
{

// Exactly one MovementHook exists per plugin module (a Services member); the plain-function
// detour thunk reaches it through this pointer. Both are module-local statics, so two plugins
// each get their own independent detour (see the header's stacking caveat).
MovementHook* g_activeHook = nullptr;
SafetyHookInline g_processMovement{};

// void CCSPlayer_MovementServices::ProcessMovement(CMoveData*): member function, so the
// detour receives `this` as the first argument under both x64 conventions.
void ProcessMovementDetour(void* movementServices, void* moveData)
{
    if (g_activeHook)
        g_activeHook->OnProcessMovement(movementServices, moveData);
    else
        g_processMovement.call<void>(movementServices, moveData);
}

}  // namespace

bool MovementHook::Install()
{
    if (_installed)
        return true;

    void* target = Engine().GameData.FindSignature("ProcessMovement");
    if (!target)
    {
        Log::Warn("MovementHook: gamedata signature 'ProcessMovement' not found; hook disabled.");
        return false;
    }

    auto hook = safetyhook::InlineHook::create(target, reinterpret_cast<void*>(&ProcessMovementDetour));
    if (!hook)
    {
        Log::Warn("MovementHook: inline detour on ProcessMovement failed to install.");
        return false;
    }

    g_processMovement = std::move(*hook);
    g_activeHook = this;
    _installed = true;
    Log::Info("Movement ProcessMovement detour installed at {}.", target);
    return true;
}

void MovementHook::Remove()
{
    if (!_installed)
        return;

    g_activeHook = nullptr;
    g_processMovement = {};  // move-assign empty: destructor of the old value unpatches
    _installed = false;
}

void MovementHook::RemoveListener(uint64_t id)
{
    _pre.Remove(id);
    _post.Remove(id);
}

int MovementHook::SlotFromMovementServices(void* movementServices) const
{
    if (!movementServices)
        return -1;

    auto& entities = Engine().Entities;
    for (int slot = 0; slot < Core::MaxPlayers; ++slot)
        if (entities.GetPlayerMovementServices(slot) == movementServices)
            return slot;

    return -1;
}

void MovementHook::OnProcessMovement(void* movementServices, void* moveData)
{
    int slot = SlotFromMovementServices(movementServices);

    for (const auto& [id, callback] : _pre.Items())
        callback(slot);

    g_processMovement.call<void>(movementServices, moveData);

    for (const auto& [id, callback] : _post.Items())
        callback(slot);
}

}  // namespace CS2Kit::Sdk
