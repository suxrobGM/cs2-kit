# Command System {#commands_guide}

[TOC]

> **Work in Progress** — The command API may change.

## Overview

The command system (`CS2Kit::Commands`) provides a framework for registering and dispatching chat commands. It includes:

- **Command** — Data struct holding command metadata and handler
- **CommandBuilder** — Fluent builder for constructing commands
- **CommandManager** — Singleton that registers commands, parses chat input, and dispatches handlers
- **ICommandCaller** — Abstract interface for the entity invoking a command (player or console)

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
        .OnExecute([](CS2Kit::Commands::ICommandCaller* caller,
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

## ICommandCaller Interface

The `ICommandCaller` interface abstracts the command invoker:

```cpp
class ICommandCaller
{
public:
    virtual ~ICommandCaller() = default;
    virtual int GetSlot() const = 0;
    virtual std::string GetName() const = 0;
    virtual void Reply(const std::string& message) = 0;
    virtual bool IsPlayer() const = 0;
};
```

Your plugin provides concrete implementations (e.g., `PlayerCaller`, `ConsoleCaller`) when dispatching commands.

## CommandResult

Handlers return a `CommandResult`:

```cpp
struct CommandResult
{
    bool Success = true;
    std::string Message;
};
```

The `Message` field is sent back to the caller via `ICommandCaller::Reply()` if non-empty.
