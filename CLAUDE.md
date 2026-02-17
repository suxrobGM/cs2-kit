# CS2-Kit — C++23 CS2 Plugin Development Library

Reusable C++23 library for building Counter-Strike 2 server plugins with Metamod:Source 2.0.

## Tech Stack

- **Language:** C++23
- **Framework:** Metamod:Source 2.0 + hl2sdk-cs2
- **Build:** Source inclusion (recommended) or standalone static library via AMBuild
- **Docs:** Doxygen + doxygen-awesome-css, deployed to GitHub Pages

## Project Structure

```text
include/CS2Kit/                 # Public API headers (#include <CS2Kit/...>)
├── CS2Kit.hpp                  # InitParams, Initialize/Shutdown/OnGameFrame API
├── Commands/                   # Command, CommandBuilder, CommandManager, ICommandCaller
├── Core/                       # Singleton, ILogger, Paths
├── Menu/                       # Menu, MenuBuilder, MenuManager
├── Sdk/                        # GameInterfaces, Entity, GameData, PlayerController,
│                               # ConVarService, GameEventService, UserMessage
└── Utils/                      # SteamId, StringUtils, TimeUtils, Translations, Log

src/                            # Implementation (.cpp) + internal headers
├── CS2Kit.cpp                  # Initialize impl (resolves SDK interfaces via ISmmAPI)
├── Commands/                   # Command dispatch implementation
├── Core/                       # ConsoleLogger, Scheduler, Paths
├── Menu/                       # MenuManager, MenuRenderer
├── Sdk/                        # Schema, SigScanner, VirtualCall, Entity, GameData, etc.
└── Utils/                      # SteamId, StringUtils, TimeUtils, Translations

gamedata/                       # Engine signatures and offsets (auto-loaded)
└── signatures.jsonc            # Platform-specific byte patterns, vtable offsets

docs/                           # Doxygen extra pages (mainpage, guides)
vendor/                         # SDK submodules (hl2sdk-cs2, mmsource-2.0, nlohmann)
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

| Interface | Purpose | Required? |
| --- | --- | --- |
| `ILogger` | Logging backend | No — built-in `ConsoleLogger` used by default |
| `ICommandCaller` | Command sender abstraction | Yes — implement per-plugin (wraps your Player type) |

## Build Commands

```bash
# Generate docs locally (requires Doxygen)
doxygen Doxyfile
# Output: build/docs/html/index.html
```

Source inclusion: compiled as part of the consuming project. Static library: `python configure.py --sdks cs2 && cd build && ambuild`.

## Documentation

- **API docs:** [suxrobgm.github.io/cs2-kit](https://suxrobgm.github.io/cs2-kit/)
- **Docs source:** `docs/` folder (Markdown pages processed by Doxygen)
- **Auto-deployed** via GitHub Actions on push to `main`
