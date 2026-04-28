# Command System {#commands_guide}

[TOC]

> **Work in Progress** — The command API may change.

## Overview

The command system (`CS2Kit::Commands`) provides a framework for registering and dispatching chat commands. It includes:

- **Command** — Data struct holding command metadata and handler
- **CommandBuilder** — Fluent builder for constructing commands
- **CommandManager** — Singleton that registers commands, parses chat input, and dispatches handlers

Handlers receive a `CS2Kit::Players::Player*` directly — no caller adapter is required.

## Registering Commands

Use `CommandBuilder` to define commands and register them with `CommandManager`:

```cpp
#include <Commands/Command.hpp>
#include <Commands/CommandManager.hpp>

auto& cmdMgr = CS2Kit::Commands::CommandManager::Instance();

cmdMgr.Register(
    CS2Kit::Commands::CommandBuilder("kick")
        .WithAliases({"k"})
        .WithDescription("Kick a player from the server")
        .WithUsage("!kick <target> [reason]")
        .RequirePermission("c")
        .WithArgs(1, 2)
        .OnExecute([](CS2Kit::Players::Player* caller,
                      const std::vector<std::string>& args) -> CS2Kit::Commands::CommandResult
        {
            // args[0] = target, args[1] = reason (optional)
            return {.Success = true, .Message = "Player kicked."};
        })
        .Build()
);
```

## CommandBuilder API

| Method | Description |
|--------|-------------|
| `CommandBuilder(name)` | Constructor — sets the primary command name |
| `.WithAliases({...})` | Alternative names (e.g., `{"k", "boot"}`) |
| `.WithDescription(desc)` | Human-readable description |
| `.WithUsage(usage)` | Usage string shown in help |
| `.RequirePermission(flags)` | Required admin flag string (e.g., `"c"` for kick) |
| `.WithArgs(min, max)` | Argument count bounds (default: 0–99) |
| `.OnExecute(handler)` | The command handler function |
| `.Build()` | Returns the constructed `Command` |

## Command Prefixes

Commands are triggered by chat messages starting with `!` or `.`:

```
!kick player1 cheating
.kick player1 cheating
```

The `CommandManager` strips the prefix and matches the remaining text against registered command names and aliases.

## Permission Checking

Set a permission callback on `CommandManager` to integrate with your admin system:

```cpp
cmdMgr.SetPermissionCallback(
    [](int slot, const std::string& requiredFlags) -> bool
    {
        // Check if the player in this slot has the required flags
        return myAdminSystem.HasPermission(slot, requiredFlags);
    }
);
```

## Caller

Command handlers receive a `CS2Kit::Players::Player*` (the same pointer returned by `PlayerManager::GetPlayerBySlot`). Use `caller->GetSteamID()` and `caller->GetName()` directly.

Server-console commands are not currently dispatched through `CommandManager` — only chat messages are. If console support is added later, the signature will be revised then.

## CommandResult

Handlers return a `CommandResult`:

```cpp
struct CommandResult
{
    bool Success = true;
    std::string Message;
};
```

By default the result is discarded after dispatch. Register a `ResultCallback` on `CommandManager` to forward `Message` somewhere — typically as a chat reply via @ref chat_guide :

```cpp
cmdMgr.SetResultCallback(
    [](CS2Kit::Players::Player* caller,
       const CS2Kit::Commands::Command& cmd,
       const CS2Kit::Commands::CommandResult& result)
    {
        if (caller && !result.Message.empty())
            CS2Kit::Utils::Chat::Print(caller->GetSlot(), result.Message);
    });
```

The callback also fires for early dispatch failures (bad arg count, permission denied) with a synthesized `CommandResult` like `{false, "Usage: !ban <target> <minutes>"}` or `{false, "You do not have permission..."}`, so every code path the caller hits produces feedback without each handler having to do it manually.
