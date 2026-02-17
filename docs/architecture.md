# Architecture {#architecture}

[TOC]

> **Work in Progress** — This architecture may change as the library evolves.

## Overview

CS2-Kit is organized into five modules, each in its own namespace under `CS2Kit`:

```
CS2Kit
├── Core        Foundational patterns and services
├── Commands    Chat command framework
├── Menu        WASD center-HTML menu system
├── Sdk         HL2SDK abstraction layer
└── Utils       General-purpose utilities
```

## Design Principles

- **Single-threaded** — All code runs on the game thread. No mutexes needed. Metamod hooks are always called from the main thread.
- **Singleton pattern** — Manager classes use the CRTP `Singleton<T>` base with pass-key idiom for safe, lazy initialization.
- **Builder pattern** — Complex objects (commands, menus) are constructed via fluent builders.
- **Minimal boilerplate** — `CS2Kit::Initialize(ismm, error, maxlen)` handles all SDK interface resolution, gamedata loading, and subsystem init. Only `ICommandCaller` must be implemented by the consumer; `ILogger` has a built-in default.

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
  └── Sdk ─────────────┘────────┘
```

- **Utils** has no internal dependencies (only standard library)
- **Core** depends on nothing within CS2-Kit
- **Commands**, **Menu**, and **Sdk** depend on Core
- **Sdk** additionally depends on Utils (for SteamID, string helpers)
- **Commands** and **Menu** are independent of each other

## Callback Conventions

CS2-Kit uses `std::function` callbacks throughout:

| Callback | Signature | Used in |
|----------|-----------|---------|
| Command handler | `CommandResult(ICommandCaller*, const vector<string>&)` | CommandBuilder |
| Menu item select | `void(int slot)` | MenuBuilder |
| Menu close | `void(int slot)` | MenuBuilder |
| Submenu factory | `shared_ptr<Menu>(int slot)` | MenuBuilder |
| Scheduler task | `void()` | Scheduler |
| Permission check | `bool(int slot, const string& flags)` | CommandManager |

## Interface Contracts

| Interface | Purpose | Required? |
|-----------|---------|-----------|
| `ILogger` | Logging backend (Info/Warn/Error) | No — built-in `ConsoleLogger` used by default |
| `ICommandCaller` | Command sender abstraction | Yes — implement per-plugin (wraps your Player type) |
