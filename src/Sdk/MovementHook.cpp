#include <CS2Kit/Core/MetamodPluginBase.hpp>
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Core/Slot.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/MovementHook.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <algorithm>
#include <cs_usercmd.pb.h>

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

    _pbOffset = Engine().GameData.GetOffset("UserCmdPB");
    if (_pbOffset < 0)
        Log::Warn("MovementHook: gamedata offset 'UserCmdPB' missing; cmd listeners get Valid=false views.");

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
    _preCmd.Remove(id);
    _postCmd.Remove(id);
    _filter.Remove(id);
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

void MovementHook::DecodeUserCmd(void* userCmd)
{
    _cmdView = {};
    if (!userCmd || _pbOffset < 0)
        return;

    const auto* pb = reinterpret_cast<const CSGOUserCmdPB*>(static_cast<char*>(userCmd) + _pbOffset);
    const auto& base = pb->base();

    _cmdView.Valid = true;
    _cmdView.ClientTick = base.client_tick();
    if (base.has_viewangles())
    {
        _cmdView.ViewPitch = base.viewangles().x();
        _cmdView.ViewYaw = base.viewangles().y();
    }
    _cmdView.ForwardMove = base.forwardmove();
    _cmdView.LeftMove = base.leftmove();
    if (base.has_buttons_pb())
    {
        _cmdView.ButtonsHeld = base.buttons_pb().buttonstate1();
        _cmdView.ButtonsChanged = base.buttons_pb().buttonstate2();
    }
    _cmdView.MouseDx = base.mousedx();
    _cmdView.MouseDy = base.mousedy();
    _cmdView.Attack1StartHistoryIndex = pb->attack1_start_history_index();
    _cmdView.Attack2StartHistoryIndex = pb->attack2_start_history_index();

    int count = std::min(base.subtick_moves_size(), UserCmdView::MaxSubtickMoves);
    _cmdView.SubtickMoveCount = count;
    for (int i = 0; i < count; ++i)
    {
        const auto& move = base.subtick_moves(i);
        _cmdView.SubtickMoves[i] = {
            .Button = move.button(),
            .Pressed = move.pressed(),
            .When = move.when(),
            .PitchDelta = move.pitch_delta(),
            .YawDelta = move.yaw_delta(),
        };
    }

    int history = std::min(pb->input_history_size(), UserCmdView::MaxInputHistory);
    _cmdView.InputHistorySampleCount = history;
    for (int i = 0; i < history; ++i)
    {
        const auto& entry = pb->input_history(i);
        auto& sample = _cmdView.InputHistorySamples[i];
        sample.TargetEntIndex = entry.target_ent_index();
        if (entry.has_view_angles())
        {
            sample.HasViewAngles = true;
            sample.ViewPitch = entry.view_angles().x();
            sample.ViewYaw = entry.view_angles().y();
        }
    }
}

void* MovementHook::Hook_RunCommandPre(void* userCmd)
{
    _preSlot = SlotFromMovementServices(META_IFACEPTR(void));
    if (!_preCmd.Empty() || !_postCmd.Empty() || !_filter.Empty())
        DecodeUserCmd(userCmd);
    // Filters edit the decoded view before anyone reads it, so pre/preCmd/postCmd listeners
    // and InputHistory all observe the same edited command.
    for (const auto& [id, filter] : _filter.Items())
        filter(_preSlot, _cmdView);
    for (const auto& [id, callback] : _pre.Items())
        callback(_preSlot);
    for (const auto& [id, callback] : _preCmd.Items())
        callback(_preSlot, _cmdView);
    RETURN_META_VALUE(MRES_IGNORED, nullptr);
}

void* MovementHook::Hook_RunCommandPost(void* /*userCmd*/)
{
    // Post always brackets the same RunCommand call as the preceding pre (movement is
    // processed one player at a time, no nesting), so reuse the pre-resolved slot and
    // the pre-decoded cmd view rather than repeating the work.
    for (const auto& [id, callback] : _post.Items())
        callback(_preSlot);
    for (const auto& [id, callback] : _postCmd.Items())
        callback(_preSlot, _cmdView);
    RETURN_META_VALUE(MRES_IGNORED, nullptr);
}

}  // namespace CS2Kit::Sdk
