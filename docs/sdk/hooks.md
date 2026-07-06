# Movement Hook & Server Commands {#sdk_hooks_guide}

[TOC]

## MovementHook

@ref CS2Kit::Sdk::MovementHook brackets one player's movement processing with two inline detours: `CCSPlayerController::ProcessUsercmds` (the usercmd batch - subtick button decomposition and the jump code's convar reads happen here) and `CCSPlayer_MovementServices::ProcessMovement` (the movement simulation). ProcessMovement alone is not enough: the subtick jump code reads `sv_autobunnyhopping` outside it, which was exactly the bug that motivated the second detour.

Listeners see one logical scope: when the calls nest, pre fires once at the outermost entry and post once at the outermost exit. That makes the pair the right place for per-player state flips (see @ref CS2Kit::Sdk::RawConVar "RawConVar") that the engine's own movement logic must observe - e.g. server-side auto-bunnyhop for a single player.

The service is a dormant `Services` member: it costs nothing until a plugin calls `Install()`, and it removes its detours on destruction.

```cpp
// Callbacks can be registered up front; they fire only once the hook is installed.
Engine().MovementHook.ListenPre([](int slot) { /* before this player's movement runs */ });
Engine().MovementHook.ListenPost([](int slot) { /* after it ran - restore state here */ });

Engine().MovementHook.Install();   // resolves the signatures and patches; no-op once installed
```

Details worth knowing:

- The detours (SafetyHook, vendored under `vendor/safetyhook/`) patch the functions themselves, so they cover every player and need no live pawn - `Install()` works from OnLoad.
- The owning slot is resolved for you (`-1` when unresolved, e.g. an instance mid-destruction).
- `GetStats()` exposes lifetime call/scope/unresolved counters - surface them through a plugin debug command for live diagnosis (bhop does this via `bhop_debug`).
- The byte patterns live in gamedata as `"ProcessUsercmds"` and `"ProcessMovement"` and **drift with CS2 updates** - each failed match disables just that detour with a logged warning (no crash). Re-verify the patterns (against SwiftlyS2/CS2Fixes gamedata) after every game update.
- Run at most one plugin that installs these detours: inline hooks stacked by separately unloadable modules are only safe to remove in reverse install order.

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
