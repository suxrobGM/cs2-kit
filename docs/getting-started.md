# Getting Started {#getting_started}

[TOC]

> **Work in Progress** — CS2-Kit is under active development. Expect breaking changes.

## Prerequisites

- **C++23** compiler (MSVC 19.38+, GCC 13+, Clang 17+)
- **HL2SDK** (hl2sdk-cs2 branch)
- **Metamod:Source 2.0**
- **AMBuild** (or your preferred build system)

## Adding CS2-Kit to Your Project

CS2-Kit is designed to be included as source files in your Metamod plugin project. There is no separate compiled library — you compile the `.cpp` files alongside your own code.

### As a Git Submodule

```sh
git submodule add https://github.com/suxrobgm/cs2-kit.git vendor/cs2-kit
```

### AMBuild Integration

In your `AMBuildScript`, add CS2-Kit source files to your build:

```python
# Add CS2-Kit sources
import os

cs2kit_root = os.path.join(builder.sourcePath, "vendor", "cs2-kit", "src")
for root, dirs, files in os.walk(cs2kit_root):
    for f in files:
        if f.endswith(".cpp"):
            binary.sources.append(os.path.relpath(os.path.join(root, f), builder.sourcePath))

# Add include path
binary.compiler.cxxincludes += [
    os.path.join(builder.sourcePath, "vendor", "cs2-kit", "src"),
]
```

## Initialization

CS2-Kit singletons must be initialized during your plugin's `Load()` method. At minimum, you need to:

1. Populate `GameInterfaces` with SDK interface pointers
2. Register a logger implementation
3. Register a path resolver

```cpp
#include <Sdk/GameInterfaces.hpp>
#include <Core/ILogger.hpp>
#include <Core/IPathResolver.hpp>

using namespace CS2Kit::Core;
using namespace CS2Kit::Sdk;

bool MyPlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    // 1. Populate SDK interfaces
    auto& gi = GameInterfaces::Instance();
    GET_V_IFACE_ANY(GetServerFactory, gi.ServerGameDLL, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_ANY(GetEngineFactory, gi.Engine, IVEngineServer2, INTERFACEVERSION_VENGINESERVER);
    GET_V_IFACE_ANY(GetEngineFactory, gi.CVar, ICvar, CVAR_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetEngineFactory, gi.SchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetEngineFactory, gi.NetworkMessages, INetworkMessages, NETWORKMESSAGES_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetFileSystemFactory, gi.GameResourceService, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
    // ... other interfaces as needed

    // 2. Register logger (implement CS2Kit::Core::ILogger)
    SetGlobalLogger(&myLogger);

    // 3. Register path resolver (implement CS2Kit::Core::IPathResolver)
    SetGlobalPathResolver(&myPathResolver);

    // 4. Load gamedata (optional, for signature scanning)
    GameData::Instance().Load("gamedata/signatures.jsonc");

    return true;
}
```

## Game Frame Hook

The scheduler and menu system require a per-tick callback. Hook `IServerGameDLL::GameFrame`:

```cpp
void MyPlugin::GameFrame(bool simulating, bool firstTick, bool lastTick)
{
    CS2Kit::Core::Scheduler::Instance().OnGameFrame();
    CS2Kit::Menu::MenuManager::Instance().OnGameFrame();
}
```

## Next Steps

- @ref commands_guide — Register chat commands
- @ref menus_guide — Build interactive menus
- @ref sdk_guide — Access entities, schema fields, and engine services
