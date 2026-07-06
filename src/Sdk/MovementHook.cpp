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
// detour thunks reach it through this pointer. All module-local statics, so two plugins
// each get their own independent detours (see the header's stacking caveat).
MovementHook* g_activeHook = nullptr;
SafetyHookInline g_processMovement{};
SafetyHookInline g_processUsercmds{};

// void CCSPlayer_MovementServices::ProcessMovement(CMoveData*): member function, so the
// detour receives `this` as the first argument under both x64 conventions.
void ProcessMovementDetour(void* movementServices, void* moveData)
{
    if (g_activeHook)
        g_activeHook->OnProcessMovement(movementServices, moveData);
    else
        g_processMovement.call<void>(movementServices, moveData);
}

// void* CCSPlayerController::ProcessUsercmds(CUserCmd*, int numcmds, bool paused, float margin)
// - argument list mirrored from CS2Fixes' detour of the same function.
void* ProcessUsercmdsDetour(void* controller, void* cmds, int numcmds, bool paused, float margin)
{
    if (g_activeHook)
        return g_activeHook->OnProcessUsercmds(controller, cmds, numcmds, paused, margin);
    return g_processUsercmds.call<void*>(controller, cmds, numcmds, paused, margin);
}

bool InstallDetour(SafetyHookInline& hook, const char* signatureName, void* detour)
{
    void* target = Engine().GameData.FindSignature(signatureName);
    if (!target)
    {
        Log::Warn("MovementHook: gamedata signature '{}' not found; that detour is disabled.", signatureName);
        return false;
    }

    auto created = safetyhook::InlineHook::create(target, detour);
    if (!created)
    {
        Log::Warn("MovementHook: inline detour on {} failed to install.", signatureName);
        return false;
    }

    hook = std::move(*created);
    Log::Info("Movement {} detour installed at {}.", signatureName, target);
    return true;
}

}  // namespace

bool MovementHook::Install()
{
    if (_installed)
        return true;

    bool movement =
        InstallDetour(g_processMovement, "ProcessMovement", reinterpret_cast<void*>(&ProcessMovementDetour));
    bool usercmds =
        InstallDetour(g_processUsercmds, "ProcessUsercmds", reinterpret_cast<void*>(&ProcessUsercmdsDetour));

    if (!movement && !usercmds)
        return false;

    g_activeHook = this;
    _installed = true;
    return true;
}

void MovementHook::Remove()
{
    if (!_installed)
        return;

    g_activeHook = nullptr;
    // Move-assign empty: the destructor of the old value unpatches.
    g_processMovement = {};
    g_processUsercmds = {};
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

int MovementHook::SlotFromController(void* controller) const
{
    if (!controller)
        return -1;

    auto& entities = Engine().Entities;
    for (int slot = 0; slot < Core::MaxPlayers; ++slot)
        if (entities.GetPlayerController(slot) == controller)
            return slot;

    return -1;
}

void MovementHook::EnterScope(int slot)
{
    if (_depth++ > 0)
        return;  // nested inside this player's outer scope - listeners already fired

    _scopeSlot = slot;
    if (slot >= 0)
        ++_stats.ScopesEntered;
    else
        ++_stats.UnresolvedSlots;

    for (const auto& [id, callback] : _pre.Items())
        callback(slot);
}

void MovementHook::ExitScope()
{
    if (--_depth > 0)
        return;

    for (const auto& [id, callback] : _post.Items())
        callback(_scopeSlot);
}

void MovementHook::OnProcessMovement(void* movementServices, void* moveData)
{
    ++_stats.MovementCalls;

    // When nested inside ProcessUsercmds this is the same player - skip the reverse lookup.
    EnterScope(_depth > 0 ? _scopeSlot : SlotFromMovementServices(movementServices));
    g_processMovement.call<void>(movementServices, moveData);
    ExitScope();
}

void* MovementHook::OnProcessUsercmds(void* controller, void* cmds, int numcmds, bool paused, float margin)
{
    ++_stats.UsercmdsCalls;

    EnterScope(SlotFromController(controller));
    void* result = g_processUsercmds.call<void*>(controller, cmds, numcmds, paused, margin);
    ExitScope();
    return result;
}

}  // namespace CS2Kit::Sdk
