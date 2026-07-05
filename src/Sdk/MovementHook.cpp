#include <CS2Kit/Core/MetamodPluginBase.hpp>
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Core/Slot.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/MovementHook.hpp>
#include <CS2Kit/Utils/Log.hpp>

PLUGIN_GLOBALVARS();

using CS2Kit::Core::Engine;

namespace CS2Kit::Sdk
{
using namespace CS2Kit::Utils;

// void* return/param stand in for the real CPlayer_MovementServices::RunCommand(CUserCmd*)
// signature - a pre/post observer never touches either. The vtable index is reconfigured
// from gamedata at install time.
SH_DECL_MANUALHOOK1(CS2Kit_MovementRunCommand, 0, 0, 0, void*, void*);

bool MovementHook::Install()
{
    if (_installed)
        return true;

    int index = Engine().GameData.GetOffset("RunCommand");
    if (index < 0)
    {
        Log::Warn("MovementHook: gamedata offset 'RunCommand' missing; hook disabled.");
        return false;
    }

    // Any live instance works: SourceHook patches the class vtable, covering all players.
    void* instance = nullptr;
    for (int slot = 0; slot < Core::MaxPlayers && !instance; ++slot)
        instance = Engine().Entities.GetPlayerMovementServices(slot);
    if (!instance)
        return false;

    SH_MANUALHOOK_RECONFIGURE(CS2Kit_MovementRunCommand, index, 0, 0);
    SH_ADD_MANUALHOOK(CS2Kit_MovementRunCommand, instance, SH_MEMBER(this, &MovementHook::Hook_RunCommandPre), false);
    SH_ADD_MANUALHOOK(CS2Kit_MovementRunCommand, instance, SH_MEMBER(this, &MovementHook::Hook_RunCommandPost), true);

    _vtable = *static_cast<void**>(instance);
    _installed = true;
    Log::Info("Movement RunCommand hook installed (vtable index {}).", index);
    return true;
}

void MovementHook::Remove()
{
    if (!_installed)
        return;

    // The instance hooked in Install() may be gone by now (map change destroys pawns).
    // SourceHook locates a manual vhook through the object's vtable pointer, so hand it a
    // stand-in object whose first pointer-sized field is that same vtable.
    void* fake = &_vtable;
    SH_REMOVE_MANUALHOOK(CS2Kit_MovementRunCommand, fake, SH_MEMBER(this, &MovementHook::Hook_RunCommandPre), false);
    SH_REMOVE_MANUALHOOK(CS2Kit_MovementRunCommand, fake, SH_MEMBER(this, &MovementHook::Hook_RunCommandPost), true);

    _installed = false;
    _vtable = nullptr;
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

void* MovementHook::Hook_RunCommandPre(void* userCmd)
{
    _preSlot = SlotFromMovementServices(META_IFACEPTR(void));
    for (const auto& [id, callback] : _pre.Items())
        callback(_preSlot, userCmd);
    RETURN_META_VALUE(MRES_IGNORED, nullptr);
}

void* MovementHook::Hook_RunCommandPost(void* /*userCmd*/)
{
    // Post always brackets the same RunCommand call as the preceding pre (movement is
    // processed one player at a time, no nesting), so reuse the pre-resolved slot rather
    // than repeating the O(players) reverse lookup.
    for (const auto& [id, callback] : _post.Items())
        callback(_preSlot);
    RETURN_META_VALUE(MRES_IGNORED, nullptr);
}

}  // namespace CS2Kit::Sdk
