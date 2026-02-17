# CS2-Kit

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://suxrobgm.github.io/cs2-kit/)

C++23 library for building Counter-Strike 2 server plugins with Metamod:Source 2.0.

> **Work in Progress** - This library is under active development. The API is unstable and subject to breaking changes. Features may be added, removed, or redesigned without notice.

## Features

- **Core** - CRTP singleton base, tick-based scheduler, ILogger interface with built-in ConsoleLogger
- **Commands** - Chat command framework with fluent builder, aliases, argument validation, and permission checks
- **Menu** - WASD-navigated center-HTML menus with builder pattern, submenu stacks, and custom layouts
- **Sdk** - Typed wrappers for HL2SDK interfaces: entities, schema fields, signature scanning, ConVars, game events, user messages
- **Utils** - SteamID conversions, string manipulation, duration parsing, JSON-based translations

## Integration

CS2-Kit supports two integration methods. Both produce a single `.dll`/`.so` plugin binary.

### Option 1: Source Inclusion (recommended)

Compile CS2-Kit sources directly into your plugin binary. Faster incremental builds.

```sh
git submodule add https://github.com/suxrobgm/cs2-kit.git vendor/cs2-kit
```

In your AMBuild, add the include path and compile the `.cpp` files:

```python
CS2KIT_DIR = os.path.join(builder.sourcePath, "vendor", "cs2-kit")
CS2KIT_INCLUDE_DIR = os.path.join(CS2KIT_DIR, "include")
CS2KIT_SRC_DIR = os.path.join(CS2KIT_DIR, "src")

# Add public headers to include path
binary.compiler.cxxincludes += [CS2KIT_INCLUDE_DIR]

# Auto-discover and compile cs2-kit sources
for root, dirs, files in os.walk(CS2KIT_SRC_DIR):
    for f in files:
        if f.endswith(".cpp"):
            binary.sources += [os.path.join(root, f)]
```

### Option 2: Static Library

Build CS2-Kit as a standalone `.lib`/`.a`, then link against it. CS2-Kit includes its own AMBuild for this:

```sh
cd vendor/cs2-kit
python configure.py --sdks cs2
cd build && ambuild
```

This produces `cs2-kit.lib` (Windows) or `cs2-kit.a` (Linux). Link it into your plugin binary and add `vendor/cs2-kit/include/` to your include path.

## Quick Start

All includes use the `<CS2Kit/...>` prefix:

```cpp
#include <CS2Kit/CS2Kit.hpp>
```

### Initialize in Plugin::Load()

```cpp
bool MyPlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    CS2Kit::InitParams params;
    params.BaseDir = ismm->GetBaseDir();
    params.LogPrefix = "MyPlugin";
    params.GameDataPath = "addons/my-plugin/gamedata/signatures.jsonc";

    // Resolve SDK interfaces via Metamod macros
    GET_V_IFACE_ANY(GetServerFactory, params.ServerGameDLL, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_ANY(GetServerFactory, params.ServerGameClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
    GET_V_IFACE_ANY(GetEngineFactory, params.GameEventSystem, IGameEventSystem, GAMEEVENTSYSTEM_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetEngineFactory, params.NetworkMessages, INetworkMessages, NETWORKMESSAGES_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetEngineFactory, params.SchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, params.GameResourceService, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetEngineFactory, params.CVar, ICvar, CVAR_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetEngineFactory, params.Engine, IVEngineServer2, INTERFACEVERSION_VENGINESERVER);

    // Required by HL2SDK's convar.cpp
    g_pCVar = params.CVar;

    if (!CS2Kit::Initialize(params))
    {
        snprintf(error, maxlen, "Failed to initialize CS2Kit");
        return false;
    }

    // Your plugin initialization here...
    return true;
}
```

### Hook Callbacks

```cpp
void MyPlugin::Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
    CS2Kit::OnGameFrame();  // Drives Scheduler + MenuManager
}

void MyPlugin::Hook_ClientDisconnect(CPlayerSlot slot, ...)
{
    CS2Kit::OnPlayerDisconnect(slot.Get());  // Cleans up menu state
}
```

### Shutdown

```cpp
bool MyPlugin::Unload(char* error, size_t maxlen)
{
    CS2Kit::Shutdown();
    return true;
}
```

## Usage Examples

### Register a Command

```cpp
#include <CS2Kit/Commands/Command.hpp>
#include <CS2Kit/Commands/CommandManager.hpp>

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
#include <CS2Kit/Menu/MenuBuilder.hpp>
#include <CS2Kit/Menu/MenuManager.hpp>

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
#include <CS2Kit/Core/Scheduler.hpp>

auto& scheduler = CS2Kit::Core::Scheduler::Instance();

// One-shot delay (5 seconds)
scheduler.Delay(5000, []() { /* execute once */ });

// Repeating timer (every 1 second)
scheduler.Repeat(1000, []() { /* execute repeatedly */ });

// Cancel a scheduled task
uint64_t id = scheduler.Delay(10000, []() {});
scheduler.Cancel(id);
```

### Custom Logger

CS2-Kit ships a built-in `ConsoleLogger` (used by default). To provide your own:

```cpp
#include <CS2Kit/Core/ILogger.hpp>

class FileLogger : public CS2Kit::Core::ILogger
{
public:
    void Info(const std::string& message) override { /* ... */ }
    void Warn(const std::string& message) override { /* ... */ }
    void Error(const std::string& message) override { /* ... */ }
};

// Pass during initialization
CS2Kit::InitParams params;
params.Logger = new FileLogger();  // CS2Kit does NOT take ownership
```

## Project Structure

```text
include/
└── CS2Kit/                Public headers (add include/ to your include path)
    ├── CS2Kit.hpp         Umbrella header: InitParams, Initialize/Shutdown API
    ├── Commands/          Command system (Command, CommandBuilder, CommandManager, ICommandCaller)
    ├── Core/              Singleton, Scheduler, ILogger, ConsoleLogger, Paths
    ├── Menu/              Menu system (Menu, MenuBuilder, MenuManager, MenuRenderer)
    ├── Sdk/               SDK wrappers (GameInterfaces, GameData, Entity, Schema, ...)
    └── Utils/             SteamId, StringUtils, TimeUtils, Translations, Log
src/                       Implementation files (.cpp)
vendor/                    SDK submodules (hl2sdk-cs2, hl2sdk-manifests, mmsource-2.0)
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
