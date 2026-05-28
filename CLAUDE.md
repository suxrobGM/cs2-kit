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
├── Commands/                   # Command, CommandBuilder, CommandManager
├── Core/                       # MetamodPluginBase, PluginInfo, Singleton, ILogger, Paths
├── Menu/                       # Menu, MenuBuilder, MenuManager
├── Players/                    # Player (identity + connection), PlayerManager (slot/steamid lookup)
├── Sdk/                        # GameInterfaces, Entity, GameData, PlayerController,
│                               # ConVarService, GameEventService, UserMessage
└── Utils/                      # Json (Serialize/Deserialize), SteamId, StringUtils, TimeUtils, Translations, Log

src/                            # Implementation (.cpp) + internal headers
├── CS2Kit.cpp                  # Initialize impl (resolves SDK interfaces via ISmmAPI)
├── Commands/                   # Command dispatch implementation
├── Core/                       # MetamodPluginBase, ConsoleLogger, Scheduler, Paths
├── Menu/                       # MenuManager, MenuRenderer
├── Players/                    # Player, PlayerManager
├── Sdk/                        # Schema, SigScanner, VirtualCall, Entity, GameData, etc.
└── Utils/                      # SteamId, StringUtils, TimeUtils, Translations (Json is header-only)

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
| Classes/Structs | `PascalCase` | `CommandManager`, `MenuOption` |
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

### Plugin Base (MetamodPluginBase)

`CS2Kit::Core::MetamodPluginBase` is the recommended plugin entry point. Subclass it, return a
`PluginInfo` from `Info()`, and implement `OnLoad()`; override `OnPlayerConnect`/`OnPlayerDisconnect`/
`OnPlayerChat`/`OnRegisterHooks` as needed. The base owns the ISmmPlugin getters, the Load/Unload
flow, the four standard SourceHook hooks, and the `PlayerManager` add/remove lifecycle.

```cpp
class MyPlugin : public CS2Kit::Core::MetamodPluginBase {
    CS2Kit::Core::PluginInfo Info() const override { return { .Name = "My Plugin", .LogTag = "MINE" }; }
    bool OnLoad(bool late) override { Defer([]{ MySystem::Shutdown(); }); return MySystem::Init(); }
};
MyPlugin g_MyPlugin;
PLUGIN_EXPOSE(MyPlugin, g_MyPlugin);  // consumer .cpp; PLUGIN_GLOBALVARS() in header
```

- `Defer(fn)` registers cleanups run LIFO on unload or failed load (guarantees `CS2Kit::Shutdown()`).
- Custom hooks: register in `OnRegisterHooks()`, pair with `Defer()` for removal. `SH_DECL_HOOK` for
  custom hooks stays in the consumer TU; the base owns the standard hooks.
- `MetamodPluginBase.cpp` carries `PLUGIN_GLOBALVARS()` so it can reference the SourceHook globals
  the consumer's `PLUGIN_EXPOSE` defines (works under source inclusion and static-lib linking).

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
    .AddButton("Option", onSelect)
    .AddToggle("Beacon", "ON", "OFF", getState, onToggle)  // live-state row
    .AddSubmenu("Sub", factory)
    .Build();
```

Every row is a typed `MenuOption` (Button, Toggle, Choice, Selector, Slider, ProgressBar, Input, Submenu). For a button whose label reflects live state, use `AddDynamicButton(getLabel, onActivate)`. See the menu guide for the full set.

## Build Commands

```bash
# Generate docs locally (requires Doxygen)
doxygen Doxyfile
# Output: build/docs/html/index.html
```

Source inclusion: compiled as part of the consuming project. Static library: `python configure.py --sdks cs2 && cd build && ambuild`.
