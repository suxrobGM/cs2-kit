# Architecture {#architecture}

[TOC]

## Overview

CS2-Kit is organized into six modules, each in its own namespace under `CS2Kit`:

```
CS2Kit
├── Core        Foundational patterns and services
├── Commands    Chat command framework
├── Menu        WASD center-HTML menu system
├── Players     Connected-player tracking
├── Sdk         HL2SDK abstraction layer
└── Utils       General-purpose utilities
```

## Design Principles

- **Single-threaded** — All code runs on the game thread. No mutexes needed. Metamod hooks are always called from the main thread.
- **Singleton pattern** — Manager classes use the CRTP `Singleton<T>` base with pass-key idiom for safe, lazy initialization.
- **Builder pattern** — Complex objects (commands, menus) are constructed via fluent builders.
- **Minimal boilerplate** — `CS2Kit::Initialize(ismm, error, maxlen)` handles all SDK interface resolution, gamedata loading, and subsystem init. Deriving from `MetamodPluginBase` removes the rest: the ISmmPlugin getters, the Load/Unload skeleton, the standard hooks, and the player lifecycle. `ILogger` has a built-in default.

## Plugin lifecycle (MetamodPluginBase)

`CS2Kit::Core::MetamodPluginBase` is the recommended entry point. It implements `ISmmPlugin`,
owns the four standard SourceHook hooks (GameFrame, client connect/disconnect, chat dispatch),
drives `PlayerManager::AddPlayer`/`RemovePlayer`, and exposes virtual callbacks (`OnLoad`,
`OnPlayerConnect`, `OnPlayerDisconnect`, `OnPlayerChat`, `OnRegisterHooks`) for plugin logic.

- **Teardown stack** — `Defer(fn)` pushes a cleanup callback; the stack runs in reverse (LIFO)
  on unload **and** on a failed load. This keeps setup and teardown adjacent in the source and
  guarantees `CS2Kit::Shutdown()` runs even when `OnLoad()` rejects the load.
- **Hook ownership** — the base owns the standard hooks (their `SH_DECL_HOOK` lives inside the
  library). Custom hooks are registered by the consumer in `OnRegisterHooks()` and paired with
  `Defer()` for removal. The consumer still provides `PLUGIN_EXPOSE` / `PLUGIN_GLOBALVARS`, which
  define the per-plugin SourceHook globals the base links against.

## CRTP Singleton

All singleton classes inherit from `CS2Kit::Core::Singleton<T>`:

```cpp
class MyManager : public Core::Singleton<MyManager>
{
public:
    explicit MyManager(Token) {}  // Required constructor

    void DoWork();

private:
    int _state = 0;
};

// Usage:
MyManager::Instance().DoWork();
```

The pass-key idiom (`Token`) prevents external code from constructing additional instances while allowing `Singleton<T>` to create the one instance.

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

## Interface Contracts

| Interface | Purpose | Required? |
|-----------|---------|-----------|
| `ILogger` | Logging backend (Info/Warn/Error) | No — built-in `ConsoleLogger` used by default |
