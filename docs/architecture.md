# Architecture {#architecture}

[TOC]

## Modules

```
CS2Kit
├── Core        Plugin base, service container, policy, scheduler, registry, effects
├── Commands    Declarative chat commands (CommandSpec)
├── Players     Player tracking, target selectors, action dispatch
├── Menu        WASD center-HTML menus + Flow wizard
├── Sdk         HL2SDK wrapper layer (entities, events, messages, gamedata)
├── Database    Async PostgreSQL + row mapping (CS2KIT_ENABLE_POSTGRES)
├── Http        Async HTTP client + JSON REST helpers
└── Utils       Translations, parsing, colors, string/time helpers
```

## Ground rules

- **Game thread only.** Metamod hooks all arrive on the main thread, and everything in the kit runs there. The two exceptions - the database worker and HTTP's pool - queue their completions and replay them on the game thread from a per-frame pump, so your callbacks never race game code.
- **No process-lifetime singletons.** Every kit service is a member of one `CS2Kit::Services`, constructed on Load and destroyed on Unload. State cannot survive a `meta reload`.
- **Data over glue.** Commands, effects, and menu rows are described as structs (`CommandSpec`, `EffectDescriptor`, context rows); the kit owns the resolve/check/dispatch/reply pipeline around them.
- **Policy is injected once.** The kit carries no admin model. Your plugin sets `Engine().Policy` in OnLoad, and every permission gate, immunity check, and command reply in the kit goes through it.

## Two containers, same lifetime

**`Engine()`** is the kit's services. `PluginBase` constructs the `Services` on Load and destroys it on Unload; members are declared in dependency order and torn down in reverse. `Engine()` asserts outside that window; `EngineOrNull()` is for late-teardown paths.

```cpp
Engine().Players.GetPlayerBySlot(slot);
Engine().Messages.Reply(slot, "done");
Engine().Schema().GetOffset("CCSPlayerPawn", "m_iHealth");   // Schema() is a method
```

**`App()`** is yours. Declare your managers in a plain struct and hand it to @ref CS2Kit::Core::PluginBase as the template argument - the base constructs it *after* the kit services are live (so member initializers may call `Engine()`), publishes it, and destroys it after your `Defer` cleanups ran:

```cpp
struct Managers
{
    ConfigManager Config;                                  // declaration order == construction order
    AdminManager Admins;
    CS2Kit::EffectManager Effects{CS2Kit::Engine().Scheduler};  // kit types are fine here
};

class MyPlugin : public CS2Kit::PluginBase<Managers> { ... };
Managers& App() { return MyPlugin::App(); }

App().Admins.IsAdmin(steamId);
```

## PluginPolicy

@ref CS2Kit::Core::PluginPolicy is the one bridge between the kit's generic machinery and your domain rules. Set it once in OnLoad:

```cpp
Engine().Policy = {
    .HasPermission = [](int64_t steamId, const std::string& perm) { return App().Admins.HasAnyPermission(steamId, perm); },
    .CanTarget     = [](Player& caller, Player& target) { return App().Admins.CanTarget(caller.GetSteamID(), target.GetSteamID()); },
    .Reply         = [](int slot, std::string_view msg) { Engine().Messages.Reply(slot, msg); },
    .Broadcast     = [](Player& caller, Player* target, const std::string& key) { App().Chat.BroadcastAction(key, ...); },
};
```

Consumers: `CommandManager` (permission gate + reply routing), the target resolver (immunity), `ActionDispatcher` and the effect dispatch helpers (permission + immunity + broadcast), context menu rows and `Flow` (row enabling, validation replies). Unset members are skipped, not crashes.

## Self-registration (`Registry<T>`)

Descriptors register themselves where they are defined instead of being `push_back`'d from a central list:

```cpp
static const bool _registered = CS2Kit::Registry<CS2Kit::CommandSpec>::Add({ ... });
// OnLoad, once:
Engine().Commands.RegisterAll(CS2Kit::Registry<CS2Kit::CommandSpec>::Items());
```

The kit is a static library, so the registry static lives inside your plugin DLL: `meta unload` discards it with the module and a reload re-runs the registrants. The one constraint: items are built during static init, before Load, so they must be data-only - no `Engine()`/`App()` at construction time (lambdas that call them later are fine). Static-init order across translation units is unspecified; sort on an explicit `Order` field when presentation order matters.

## Teardown

`Defer(fn)` pushes a cleanup onto a LIFO stack that runs on unload **and** on a failed load, so setup and teardown sit next to each other and a rejected `OnLoad` never leaks initialized subsystems. The base's own order on unload: your `OnUnload`, the Defer stack, manager destruction, `CS2Kit::Shutdown()`, `Services` destruction.

## The frame pump

The plugin's GameFrame hook calls `CS2Kit::OnGameFrame()`, which ticks exactly one thing: the @ref CS2Kit::Core::Scheduler. Everything per-frame - menu input, HTTP completions, database completions - self-registers a `Scheduler::EveryFrame` timer, so there is no hardcoded pump list to keep in sync.

## Module dependencies

- **Utils** stands alone (standard library only)
- **Core** depends on nothing else in the kit
- **Commands**, **Menu**, **Players**, **Sdk** sit on Core (+ Utils)
- **Database** is Utils + libpqxx, compiled only under `CS2KIT_ENABLE_POSTGRES`
- **Http** wraps CPR; completions ride the scheduler pump

## Interface contracts

| Interface | Purpose | Required? |
|-----------|---------|-----------|
| `ILogger` | Logging backend (Info/Warn/Error) | No - a built-in `ConsoleLogger` is the default |
