#include <CS2Kit/Sdk/UserCmd.hpp>
#include <cs_usercmd.pb.h>
#include <format>

namespace CS2Kit::Sdk
{

namespace
{

// Engine wrapper around the protobuf command (not a schema class). The 0x10 head holds the
// vtable + command number; SwiftlyS2 reads the PB at +0x10 and CS2Fixes pads the same.
struct EngineUserCmd
{
    char _head[0x10];
    CSGOUserCmdPB Cmd;
};

// Full engine stride, for indexing the cmds array ProcessUsercmds receives (CS2Fixes layout).
struct EngineUserCmdFull
{
    char _head[0x10];
    CSGOUserCmdPB Cmd;
    char _tail[0x38];
#ifdef _WIN32
    char _tailWindows[0x8];
#endif
};

}  // namespace

bool InjectSubtickPress(void* userCmd, uint64_t button, bool pressed, float when)
{
    if (!userCmd)
        return false;

    auto* cmd = static_cast<EngineUserCmd*>(userCmd);
    CBaseUserCmdPB* base = cmd->Cmd.mutable_base();

    CSubtickMoveStep* step = base->add_subtick_moves();
    step->set_button(button);
    step->set_pressed(pressed);
    step->set_when(when);

    // Keep the command's button masks consistent with the injected event: [1] is the
    // changed-this-tick (edge) mask real presses always carry - input processing keys off it,
    // with the subtick step supplying the within-tick timing.
    CInButtonStatePB* buttons = base->mutable_buttons_pb();
    buttons->set_buttonstate2(buttons->buttonstate2() | button);
    if (pressed)
        buttons->set_buttonstate1(buttons->buttonstate1() | button);
    else
        buttons->set_buttonstate1(buttons->buttonstate1() & ~button);
    return true;
}

bool HasSubtickPress(void* userCmd, uint64_t button)
{
    if (!userCmd)
        return false;

    const auto* cmd = static_cast<EngineUserCmd*>(userCmd);
    if (!cmd->Cmd.has_base())
        return false;

    for (const CSubtickMoveStep& step : cmd->Cmd.base().subtick_moves())
        if (step.button() == button && step.pressed())
            return true;

    return false;
}

std::string DescribeSubtickMoves(void* userCmd, uint64_t button)
{
    if (!userCmd)
        return {};

    const auto* cmd = static_cast<EngineUserCmd*>(userCmd);
    if (!cmd->Cmd.has_base())
        return {};

    std::string result;
    for (const CSubtickMoveStep& step : cmd->Cmd.base().subtick_moves())
    {
        if (button != 0 && step.button() != button)
            continue;
        if (!result.empty())
            result += ' ';
        result += std::format("btn{}{}@{:.3f}", step.button(), step.pressed() ? '+' : '-', step.when());
    }
    return result;
}

void* UserCmdAt(void* cmds, int index)
{
    if (!cmds || index < 0)
        return nullptr;
    return static_cast<EngineUserCmdFull*>(cmds) + index;
}

}  // namespace CS2Kit::Sdk
