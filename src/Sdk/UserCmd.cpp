#include <CS2Kit/Sdk/UserCmd.hpp>
#include <cs_usercmd.pb.h>

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

}  // namespace

bool InjectSubtickPress(void* userCmd, uint64_t button, bool pressed, float when)
{
    if (!userCmd)
        return false;

    auto* cmd = static_cast<EngineUserCmd*>(userCmd);
    CSubtickMoveStep* step = cmd->Cmd.mutable_base()->add_subtick_moves();
    step->set_button(button);
    step->set_pressed(pressed);
    step->set_when(when);
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

}  // namespace CS2Kit::Sdk
