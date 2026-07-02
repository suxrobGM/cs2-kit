# Architecture {#architecture}

[TOC]

## Overview

CS2-Kit is organized into eight modules, each in its own namespace under `CS2Kit`:

```
CS2Kit
├── Core        Foundational patterns and services
├── Commands    Chat command framework
├── Database    Optional PostgreSQL client + migrations (CS2KIT_ENABLE_POSTGRES)
├── Http        Async HTTP client + JSON REST helpers
├── Menu        WASD center-HTML menu system
├── Players     Connected-player tracking, target resolution, action dispatch
├── Sdk         HL2SDK abstraction layer
└── Utils       General-purpose utilities
```

## Design Principles

- **Single-threaded** - All code runs on the game thread. No mutexes needed. Metamod hooks are always called from the main thread.
- **Service container** - CS2-Kit's services live in one `CS2Kit::Core::Services` instance, constructed on Load and destroyed on Unload, reached via the `Engine()` accessor. No process-lifetime singletons - state cannot leak across `meta unload` / `meta reload`.
- **Builder pattern** - Complex objects (commands, menus) are constructed via fluent builders.
- **Minimal boilerplate** - `CS2Kit::Initialize(ismm, error, maxlen)` handles all SDK interface resolution, gamedata loading, and subsystem init. Deriving from `MetamodPluginBase` removes the rest: the ISmmPlugin getters, the Load/Unload skeleton, the standard hooks, and the player lifecycle. `ILogger` has a built-in default.

## Plugin lifecycle (MetamodPluginBase)

`CS2Kit::Core::MetamodPluginBase` is the recommended entry point. It implements `ISmmPlugin`,
owns the four standard SourceHook hooks (GameFrame, client connect/disconnect, chat dispatch),
drives `PlayerManager::AddPlayer`/`RemovePlayer`, and exposes virtual callbacks (`OnLoad`,
`OnPlayerConnect`, `OnPlayerDisconnect`, `OnPlayerChat`, `OnRegisterHooks`) for plugin logic.

- **Teardown stack** - `Defer(fn)` pushes a cleanup callback; the stack runs in reverse (LIFO)
  on unload **and** on a failed load. This keeps setup and teardown adjacent in the source and
  guarantees `CS2Kit::Shutdown()` runs even when `OnLoad()` rejects the load.
- **Hook ownership** - the base owns the standard hooks (their `SH_DECL_HOOK` lives inside the
  library). Custom hooks are registered by the consumer in `OnRegisterHooks()` and paired with
  `Defer()` for removal. The consumer still provides `PLUGIN_EXPOSE` / `PLUGIN_GLOBALVARS`, which
  define the per-plugin SourceHook globals the base links against.

## Service container (`Engine()`)

CS2-Kit's own services - `PlayerManager`, `CommandManager`, `MenuManager`, the SDK wrappers,
`Translations`, and the rest - are members of a single `CS2Kit::Core::Services` instance. The base
constructs one `Services` on Load and destroys it on Unload (members are built in dependency order
and torn down in reverse, RAII), so service state cannot leak across `meta unload` / `meta reload`.
There are no process-lifetime singletons and no `::Instance()` accessors.

Reach a service through the free `Engine()` accessor (`Engine()` asserts if called outside a Load/Unload
window; `EngineOrNull()` returns the active `Services*` or `nullptr` for late-teardown paths):

```cpp
#include <CS2Kit/Core/Services.hpp>
using CS2Kit::Core::Engine;

Engine().Players.GetPlayerBySlot(slot);   // member field
Engine().Commands.Register(...);          // member field
Engine().Schema().GetOffset(...);         // Schema() is a method
```

A consumer plugin keeps its **own** managers the same way: declare them in a plain struct
(admin-system calls it `Managers`), construct it in `OnLoad`, drop it on unload, and reach it via a
free accessor (admin-system's is `App()`). It does not derive from any base.

```cpp
struct Managers {
    Core::ConfigManager Config;
    Admin::AdminManager  Admins;     // declaration order == construction order; destroyed in reverse
    CS2Kit::Core::EffectManager Effects{CS2Kit::Core::Engine().Scheduler};  // kit types work here too
};
Managers& App();                     // returns the live struct; valid only between OnLoad and unload

// Usage:
App().Admins.IsAdmin(steamId);
```

## Module Dependencies

```
Utils ──────────────────────────┐
  │                             │
Core ──────────────────┐        │
  │                    │        │
  ├── Commands ────────┤        │
  │                    │        │
  ├── Menu ────────────┤        │
  │                    │        │
  ├── Players ─────────┤────────┤
  │                    │        │
  └── Sdk ─────────────┘────────┘
```

- **Utils** has no internal dependencies (only standard library)
- **Core** depends on nothing within CS2-Kit
- **Commands**, **Menu**, **Players**, and **Sdk** depend on Core
- **Players** and **Sdk** additionally depend on Utils (for SteamID, string helpers)
- **Commands**, **Menu**, and **Players** are independent of each other
- **Database** depends only on Utils (logging) plus libpqxx; the whole module compiles only when
  `CS2KIT_ENABLE_POSTGRES` is ON
- **Http** wraps CPR; `HttpClient` lives in `Services` and its completions dispatch from the
  `OnGameFrame` pump

## Callback Conventions

CS2-Kit uses `std::function` callbacks throughout:

| Callback | Signature | Used in |
|----------|-----------|---------|
| Command handler | `CommandResult(Players::Player*, const vector<string>&)` | CommandBuilder |
| Menu button activate | `void(int slot)` | MenuBuilder (`AddButton`, `AddDynamicButton`) |
| Menu toggle get / flip | `bool(int) / void(int)` | MenuBuilder (`AddToggle`) |
| Menu choice get / set / commit | `int(int) / void(int, int) / void(int, const T&)` | MenuBuilder (`AddChoice`, `AddSelector`) |
| Menu slider get / set | `int(int) / void(int, int)` | MenuBuilder (`AddSlider`) |
| Menu input get / validate | `string(int) / bool(int, string_view)` | MenuBuilder (`AddInput`) |
| Menu close | `void(int slot)` | MenuBuilder |
| Submenu factory | `shared_ptr<Menu>(int slot)` | MenuBuilder (`AddSubmenu`) |
| Chat input capture | `bool(int slot, string_view text)` | ChatInputCapture (`BeginCapture`) |
| Scheduler task | `void()` | Scheduler |
| Permission check | `bool(int slot, const string& flags)` | CommandManager |
| Target policy (`CanTargetFn`) | `bool(Player& caller, Player& target)` | TargetResolver, ActionDispatcher |
| Action body | `OptKey(const ActionContext&)` | ActionDispatcher (`Action`, `ParamAction`) |
| HTTP completion | `void(const HttpResult&)` | HttpClient (`Post`) |

## Interface Contracts

| Interface | Purpose | Required? |
|-----------|---------|-----------|
| `ILogger` | Logging backend (Info/Warn/Error) | No - built-in `ConsoleLogger` used by default |
