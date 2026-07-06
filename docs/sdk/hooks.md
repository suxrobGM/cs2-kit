# Movement Hook & Server Commands {#sdk_hooks_guide}

[TOC]

## MovementHook

@ref CS2Kit::Sdk::MovementHook is an inline detour on `CCSPlayer_MovementServices::ProcessMovement` - the per-tick, per-player movement simulation (jump, stamina, friction, velocity all run inside). Pre/post callbacks bracket exactly one player's movement code, which makes the pair the right place for per-player state flips (see @ref CS2Kit::Sdk::RawConVar "RawConVar") that the engine's own movement logic must observe - e.g. server-side auto-bunnyhop for a single player.

The service is a dormant `Services` member: it costs nothing until a plugin calls `Install()`, and it removes its detour on destruction.

```cpp
// Callbacks can be registered up front; they fire only once the hook is installed.
Engine().MovementHook.ListenPre([](int slot) { /* before this player's movement runs */ });
Engine().MovementHook.ListenPost([](int slot) { /* after it ran - restore state here */ });

Engine().MovementHook.Install();   // resolves the signature and patches; no-op once installed
```

Details worth knowing:

- The detour (SafetyHook, vendored under `vendor/safetyhook/`) patches the function itself, so it covers every player and needs no live pawn - `Install()` works from OnLoad.
- The owning slot is resolved for you (`-1` when unresolved, e.g. an instance mid-destruction).
- The byte pattern lives in gamedata as `"ProcessMovement"` and **drifts with CS2 updates** - a failed match disables the hook with a logged warning (no crash). Re-verify the pattern (against SwiftlyS2/CS2Fixes gamedata) after every game update.
- Run at most one plugin that installs this detour: inline hooks stacked by separately unloadable modules are only safe to remove in reverse install order.

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
