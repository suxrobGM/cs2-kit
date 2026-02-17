# CS2-Kit

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://suxrobgm.github.io/cs2-kit/)

C++23 library for building Counter-Strike 2 server plugins with Metamod:Source 2.0.

> **Work in Progress** — This library is under active development. The API is unstable and subject to breaking changes. Features may be added, removed, or redesigned without notice.

## Features

- **Core** — CRTP singleton base, tick-based scheduler, logging and path resolution interfaces
- **Commands** — Chat command framework with fluent builder, aliases, argument validation, and permission checks
- **Menu** — WASD-navigated center-HTML menus with builder pattern, submenu stacks, and custom layouts
- **Sdk** — Typed wrappers for HL2SDK interfaces: entities, schema fields, signature scanning, ConVars, game events, user messages
- **Utils** — SteamID conversions, string manipulation, duration parsing, JSON-based translations

## Quick Start

Add CS2-Kit as a Git submodule:

```sh
git submodule add https://github.com/suxrobgm/cs2-kit.git vendor/cs2-kit
```

Include the source files in your build and add `vendor/cs2-kit/src` to your include paths. See the [Getting Started](https://suxrobgm.github.io/cs2-kit/getting_started.html) guide for detailed integration instructions.

## Usage Examples

### Register a Command

```cpp
#include <Commands/Command.hpp>
#include <Commands/CommandManager.hpp>

using namespace CS2Kit::Commands;

auto& cmdMgr = CommandManager::Instance();

cmdMgr.Register(
    CommandBuilder("kick")
        .WithAliases({"k"})
        .RequirePermission("c")
        .WithArgs(1, 2)
        .OnExecute([](ICommandCaller* caller,
                      const std::vector<std::string>& args) -> CommandResult
        {
            return {.Success = true, .Message = "Player kicked."};
        })
        .Build()
);
```

### Build a Menu

```cpp
#include <Menu/MenuBuilder.hpp>
#include <Menu/MenuManager.hpp>

using namespace CS2Kit::Menu;

auto menu = MenuBuilder("Admin Panel")
    .AddItem("Kick Player", [](int slot) { /* ... */ })
    .AddItem("Ban Player", [](int slot) { /* ... */ })
    .AddSubmenu("Settings", [](int slot) {
        return MenuBuilder("Settings")
            .AddItem("Option A", [](int slot) { /* ... */ })
            .Build();
    })
    .Build();

MenuManager::Instance().OpenMenu(playerSlot, menu);
```

### Schedule a Task

```cpp
#include <Core/Scheduler.hpp>

auto& scheduler = CS2Kit::Core::Scheduler::Instance();

// One-shot delay (5 seconds)
scheduler.Delay(5000, []() { /* execute once */ });

// Repeating timer (every 1 second)
scheduler.Repeat(1000, []() { /* execute repeatedly */ });

// Cancel a scheduled task
uint64_t id = scheduler.Delay(10000, []() {});
scheduler.Cancel(id);
```

## Project Structure

```text
src/
├── Commands/          Command system (Command, CommandBuilder, CommandManager)
├── Core/              Singleton, Scheduler, ILogger, IPathResolver
├── Menu/              Menu system (Menu, MenuBuilder, MenuManager, MenuRenderer)
├── Sdk/               SDK wrappers (GameInterfaces, GameData, Entity, Schema, ...)
└── Utils/             SteamId, StringUtils, TimeUtils, Translations
```

## Documentation

Full API documentation is available at **[suxrobgm.github.io/cs2-kit](https://suxrobgm.github.io/cs2-kit/)**.

Documentation is auto-generated from source using [Doxygen](https://www.doxygen.nl/) with the [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) theme.

To build docs locally:

```sh
doxygen Doxyfile
# Open build/docs/html/index.html
```

## Contributing

CS2-Kit is a work in progress. Contributions are welcome, but please be aware that the API is actively evolving and your changes may need to be adapted as the library matures.

## License

[MIT](LICENSE)
