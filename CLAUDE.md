# CS2-Kit — C++23 CS2 Plugin Development Library

## Project Overview

Reusable C++23 library for building Counter-Strike 2 server plugins with Metamod:Source 2.0. Extracted from the [admin-system](https://github.com/m9snoi/admin-system) project to be a standalone, general-purpose toolkit.

**Status:** Work in progress — API is unstable and subject to breaking changes.

## Tech Stack

- **Language:** C++23
- **Framework:** Metamod:Source 2.0 + hl2sdk-cs2
- **Build:** No standalone build — compiled as source by consuming projects (e.g., via AMBuild)
- **Docs:** Doxygen + doxygen-awesome-css, deployed to GitHub Pages

## Project Structure

```text
src/
├── Commands/
│   ├── Command.hpp/cpp         # Command struct + CommandBuilder (fluent API)
│   ├── CommandManager.hpp/cpp  # Registration, prefix handling (!/.), dispatch
│   └── ICommandCaller.hpp      # Abstract command invoker interface
├── Core/
│   ├── Singleton.hpp           # CRTP singleton base with pass-key idiom (header-only)
│   ├── Scheduler.hpp/cpp       # Tick-based task scheduler (Delay, Repeat, NextTick, Cancel)
│   ├── ILogger.hpp + Logger.cpp        # Logger interface + global accessor
│   └── IPathResolver.hpp + PathResolver.cpp  # Path resolution interface
├── Menu/
│   ├── Menu.hpp                # Data structures (Menu, MenuItem, MenuLayout, PlayerMenuState)
│   ├── MenuBuilder.hpp         # Fluent builder pattern (header-only)
│   ├── MenuManager.hpp/cpp     # WASD menu lifecycle, input debounce, menu stack
│   ├── MenuRenderer.hpp/cpp    # HTML rendering (3-section layout)
│   └── IMenuIO.hpp             # Menu I/O interface (buttons, center HTML)
├── Sdk/
│   ├── GameInterfaces.hpp      # Centralized SDK interface pointer holder (header-only)
│   ├── GameData.hpp/cpp        # Signature/offset JSON loader
│   ├── Entity.hpp/cpp          # Entity system access, button flags
│   ├── Schema.hpp/cpp          # Runtime schema field offset resolution with caching
│   ├── SigScanner.hpp/cpp      # Byte-pattern signature scanning + RIP-relative resolution
│   ├── UserMessage.hpp/cpp     # MessageSystem singleton (chat, center HTML)
│   ├── PlayerController.hpp/cpp  # Typed CCSPlayerController wrapper
│   ├── ConVarService.hpp/cpp   # ConVar read/write with change listeners
│   ├── GameEventService.hpp/cpp  # Event creation, firing, listener registration
│   └── VirtualCall.hpp         # Virtual function call template (header-only)
└── Utils/
    ├── Log.hpp                 # Format-based logging templates (header-only)
    ├── SteamId.hpp/cpp         # SteamID conversions (64-bit, SteamID2, SteamID3)
    ├── StringUtils.hpp/cpp     # String manipulation, target parsing (@all, #3, name)
    ├── TimeUtils.hpp/cpp       # Timestamps, duration parsing/formatting
    └── Translations.hpp/cpp    # JSON-based i18n system

docs/                           # Doxygen extra pages (mainpage, guides)
vendor/
└── doxygen-awesome-css/        # Doxygen theme (submodule)
Doxyfile                        # Doxygen configuration
.github/workflows/docs.yml     # Auto-deploy docs to GitHub Pages
```

## Code Conventions

### Naming (C# style)

| Element | Convention | Example |
| --- | --- | --- |
| Namespaces | `PascalCase`, nested | `CS2Kit::Commands` |
| Classes/Structs | `PascalCase` | `CommandManager`, `MenuItem` |
| Methods | `PascalCase` | `OpenMenu()`, `FindSignature()` |
| Member variables | `_camelCase` | `_timers`, `_nextId` |
| Constants | `PascalCase` | `MaxPlayers`, `InputDebounceMs` |
| Parameters/locals | `camelCase` | `steamId`, `targetSlot` |

### C++ Style

- **C++17 nested namespaces:** `namespace CS2Kit::Sdk { ... }`
- **`.hpp` headers** (not `.h`)
- **CRTP Singleton** with pass-key idiom (`Token` struct)
- **No mutexes** — all code runs on the game thread
- **Designated initializers** for struct construction
- **`std::format`** for string formatting
- **`std::function`** for all callbacks
- **Builder pattern** for Menu and Command construction

## Key Design Patterns

### Singleton (CRTP)

All manager classes use the pass-key idiom:

```cpp
class MyManager : public CS2Kit::Core::Singleton<MyManager>
{
public:
    explicit MyManager(Token) {}
};
// Usage: MyManager::Instance().DoWork();
```

### Builder Pattern

```cpp
// Commands
CS2Kit::Commands::CommandBuilder("kick")
    .WithAliases({"k"})
    .RequirePermission("c")
    .WithArgs(1, 2)
    .OnExecute(handler)
    .Build();

// Menus
CS2Kit::Menu::MenuBuilder("Title")
    .AddItem("Option", onSelect)
    .AddSubmenu("Sub", factory)
    .Build();
```

### Interface Contracts

Consuming plugins must implement these interfaces:

| Interface | Purpose | Registration |
| --- | --- | --- |
| `ILogger` | Logging backend | `SetGlobalLogger()` |
| `IPathResolver` | Resolve config paths | `SetGlobalPathResolver()` |
| `IMenuIO` | Button reading, HTML display | `MenuManager::SetIO()` |
| `ICommandCaller` | Command sender abstraction | Per-command invocation |

## Build Commands

```bash
# Generate docs locally (requires Doxygen)
doxygen Doxyfile
# Output: build/docs/html/index.html
```

No standalone build — the library is compiled as part of the consuming project.

## Documentation

- **API docs:** [suxrobgm.github.io/cs2-kit](https://suxrobgm.github.io/cs2-kit/)
- **Docs source:** `docs/` folder (Markdown pages processed by Doxygen)
- **Auto-deployed** via GitHub Actions on push to `main`
