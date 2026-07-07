# Movement Hook & Server Commands {#sdk_hooks_guide}

[TOC]

## MovementHook

@ref CS2Kit::Sdk::MovementHook is a manual vtable hook on `CPlayer_MovementServices::RunCommand` - the per-tick, per-player movement entry point. Pre/post callbacks bracket exactly one player's movement processing, which makes the pair the right place for per-player state flips (see @ref CS2Kit::Sdk::RawConVar "RawConVar").

The service is a dormant `Services` member: it costs nothing until a plugin calls `Install()`, and it removes its hook on destruction.

```cpp
// Callbacks can be registered up front; they fire only once the hook is installed.
Engine().MovementHook.ListenPre([](int slot) { /* before this player's movement runs */ });
Engine().MovementHook.ListenPost([](int slot) { /* after it ran - restore state here */ });

// Install needs a live movement-services instance (a spawned pawn), so call it lazily
// and treat false as "retry later" - a PlayerSpawn listener is the natural place:
Engine().Events.Listen<Events::PlayerSpawn>([](const Events::PlayerSpawn&) {
    Engine().MovementHook.Install();   // no-op once installed
});
```

Details worth knowing:

- SourceHook patches the class vtable, so hooking any one instance covers every player.
- The owning slot is resolved for you (`-1` when unresolved, e.g. an instance mid-destruction).
- Removal works even after the hooked instance died (map change): the service keeps the vtable pointer and hands SourceHook a stand-in object.
- The vtable index lives in gamedata as `"RunCommand"` and **drifts with CS2 updates** - a wrong index calls an unrelated vfunc and crashes. Re-verify it (against SwiftlyS2/CS2Fixes gamedata) after every game update.

### Cmd listeners: reading the usercmd

`ListenPreCmd`/`ListenPostCmd` additionally hand you a @ref CS2Kit::Sdk::UserCmdView - the command's viewangles, held/changed button masks, raw mouse deltas, and per-subtick pitch/yaw deltas, decoded once per RunCommand from the `CSGOUserCmdPB` payload:

```cpp
Engine().MovementHook.ListenPreCmd([](int slot, const CS2Kit::UserCmdView& cmd) {
    if (!cmd.Valid)
        return;  // null usercmd or missing gamedata offset
    // cmd.ViewYaw, cmd.MouseDx, cmd.ButtonsHeld, cmd.SubtickMoves[0].YawDelta, ...
});
```

The decode happens only while at least one cmd listener is registered; plain `ListenPre`/`ListenPost` stay free of it. The payload's byte offset inside the `CUserCmd` wrapper lives in gamedata as `"UserCmdPB"` (cross-checked against CS2Fixes and SwiftlyS2) and, like the vtable index, **must be re-verified after CS2 updates** - a missing offset degrades to `Valid=false` views rather than crashing, but a *stale* one reads garbage.

### InputHistoryService: lookback over recent usercmds

@ref CS2Kit::Sdk::InputHistoryService (`Engine().InputHistory`) is an opt-in per-slot ring buffer of the decoded views - for plugins that need "what did this player's aim do over the last N ticks" (anti-cheat, movement analytics) without wiring their own buffers:

```cpp
Engine().InputHistory.Enable(128);                    // keep ~2s at 64 tick
int n = Engine().InputHistory.Count(slot);
const auto& newest = Engine().InputHistory.At(slot, 0);  // At(slot, ago)
```

History for a slot resets automatically when its player joins or leaves (via @ref CS2Kit::Players::PlayerManager::ListenSlotChange, which is also the backing feed for the generic @ref CS2Kit::Players::PerSlot container). The MovementHook must still be installed for samples to flow.

## ServerCommand

@ref CS2Kit::Sdk::ServerCommand is a RAII tier1 `ConCommand`: registered on construction, unregistered on destruction, with a `std::function` handler that runs on the game thread.

```cpp
class MyManager
{
    std::optional<CS2Kit::ServerCommand> _cmd;

    void Initialize()
    {
        _cmd.emplace("myplugin_do", "Do the thing: myplugin_do <steamid64>",
                     [this](const CCommand& args) { /* args.ArgC(), args.Arg(1), ... */ });
    }
};
```

Beyond console/cfg use, this is the standard **cross-plugin surface**: plugins are isolated modules that cannot share managers, so a feature one plugin should drive in another is exposed as a server command and invoked via `Engine().ConVars.ExecuteServerCommand("myplugin_do 765...")`. When the providing plugin is absent the engine just logs "Unknown command" - graceful degradation for free. (This is how admin-system's Bunnyhop effect drives the bhop plugin's `bhop_player` command.)

Construct only while the plugin is loaded (ICvar must be live) - typically as a manager member, so unload unregisters it automatically.
