#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/UserCmd.hpp>
#include <CS2Kit/Sdk/UserCmdHook.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <safetyhook.hpp>

using CS2Kit::Core::Engine;

namespace CS2Kit::Sdk
{
using namespace CS2Kit::Utils;

namespace
{

// One detour per plugin .so: each plugin carries its own kit, so these statics are per-instance.
safetyhook::InlineHook g_processUsercmdsHook;
UserCmdHook* g_activeUserCmdHook = nullptr;

void* DetourProcessUsercmds(void* controller, void* cmds, int numcmds, bool paused, float margin)
{
    auto* self = g_activeUserCmdHook;
    if (self)
        self->Dispatch(controller, cmds, numcmds, true);

    void* result = g_processUsercmdsHook.call<void*>(controller, cmds, numcmds, paused, margin);

    if (self)
        self->Dispatch(controller, cmds, numcmds, false);
    return result;
}

}  // namespace

bool UserCmdHook::Install()
{
    if (_installed)
        return true;

    void* target = Engine().GameData.FindSignature("ProcessUsercmds");
    if (!target)
    {
        Log::Warn("UserCmdHook: 'ProcessUsercmds' signature unresolved; hook disabled.");
        return false;
    }

    g_processUsercmdsHook = safetyhook::create_inline(target, reinterpret_cast<void*>(&DetourProcessUsercmds));
    if (!g_processUsercmdsHook)
    {
        Log::Warn("UserCmdHook: inline hook on ProcessUsercmds failed.");
        return false;
    }

    g_activeUserCmdHook = this;
    _installed = true;
    Log::Info("ProcessUsercmds detour installed at {:#x}.", reinterpret_cast<uintptr_t>(target));
    return true;
}

void UserCmdHook::Remove()
{
    if (!_installed)
        return;

    g_activeUserCmdHook = nullptr;
    g_processUsercmdsHook = {};  // SafetyHook restores the original bytes on destruction
    _installed = false;
}

void UserCmdHook::RemoveListener(uint64_t id)
{
    _cmd.Remove(id);
    _pre.Remove(id);
    _post.Remove(id);
}

void UserCmdHook::Dispatch(void* controller, void* cmds, int numcmds, bool pre)
{
    // Controllers occupy entity indices 1..MaxPlayers, so index-1 is the slot.
    int slot = Engine().Entities.GetEntityIndex(static_cast<CEntityInstance*>(controller)) - 1;

    if (pre)
    {
        for (const auto& [id, callback] : _pre.Items())
            callback(slot);

        for (int i = 0; i < numcmds; ++i)
        {
            void* userCmd = UserCmdAt(cmds, i);
            for (const auto& [id, callback] : _cmd.Items())
                callback(slot, userCmd);
        }
    }
    else
    {
        for (const auto& [id, callback] : _post.Items())
            callback(slot);
    }
}

}  // namespace CS2Kit::Sdk
