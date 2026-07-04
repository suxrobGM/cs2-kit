#include "Managers.hpp"

#include <CS2Kit/Api.hpp>

using namespace CS2Kit::Commands;

// Commands self-register into the Registry at their definition site; Plugin.cpp ingests
// them once in OnLoad. Add more specs here (or in new .cpp files) and they are picked up
// automatically - no central registration list to maintain.
static const bool _pingRegistered = CS2Kit::Registry<CS2Kit::CommandSpec>::Add({
    .Name = "ping",
    .Description = "Check that the plugin is alive.",
    .Usage = "!ping",
    .Handler = [](CommandContext& c) { return c.Ok("cmd.pong"); },
});
