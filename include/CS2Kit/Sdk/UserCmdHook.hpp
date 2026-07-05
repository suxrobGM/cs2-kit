#pragma once

#include <CS2Kit/Core/CallbackRegistry.hpp>
#include <cstdint>
#include <functional>

namespace CS2Kit::Sdk
{

/**
 * @brief Inline detour on CCSPlayerController::ProcessUsercmds - where the engine ingests a
 * player's user commands each tick, BEFORE subtick input events are parsed into internal
 * input state.
 *
 * This is the only point where mutating a command changes what the engine acts on: by
 * MovementHook/RunCommand time the subtick events have already been consumed (verified live -
 * commands read back empty there even during real jump presses). CS2Fixes performs its
 * usercmd surgery at this same spot.
 *
 * Not a vtable hook: the target is resolved from the gamedata signature "ProcessUsercmds" and
 * detoured with the vendored SafetyHook. Signatures drift with CS2 updates - Install() fails
 * soft (warn + false) when unresolved, leaving the hook inert.
 */
class UserCmdHook
{
public:
    /** One call per command, before the engine parses it. @p userCmd is mutable via the
     *  Sdk/UserCmd.hpp helpers. @p slot may be -1 if the owning controller is unresolved. */
    using CmdCallback = std::function<void(int slot, void* userCmd)>;
    /** Around the engine's processing of one player's command batch. */
    using SlotCallback = std::function<void(int slot)>;

    UserCmdHook() = default;
    ~UserCmdHook() { Remove(); }
    UserCmdHook(const UserCmdHook&) = delete;
    UserCmdHook& operator=(const UserCmdHook&) = delete;

    /** Install the detour. False when the signature is unresolved or hooking failed. */
    bool Install();
    bool Installed() const { return _installed; }
    void Remove();

    uint64_t ListenCmd(CmdCallback callback) { return _cmd.Add(std::move(callback)); }
    uint64_t ListenPre(SlotCallback callback) { return _pre.Add(std::move(callback)); }
    uint64_t ListenPost(SlotCallback callback) { return _post.Add(std::move(callback)); }
    void RemoveListener(uint64_t id);

    /** Internal: dispatch from the detour thunk. */
    void Dispatch(void* controller, void* cmds, int numcmds, bool pre);

private:
    Core::CallbackRegistry<CmdCallback> _cmd;
    Core::CallbackRegistry<SlotCallback> _pre;
    Core::CallbackRegistry<SlotCallback> _post;
    bool _installed = false;
};

}  // namespace CS2Kit::Sdk
