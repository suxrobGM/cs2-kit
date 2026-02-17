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

## Initialization

CS2Kit handles SDK interface resolution, gamedata loading, and subsystem initialization internally. Just pass the Metamod `ISmmAPI` pointer:

```cpp
#include <CS2Kit/CS2Kit.hpp>

bool MyPlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    CS2Kit::InitParams params;
    params.LogPrefix = "MyPlugin";

    if (!CS2Kit::Initialize(ismm, error, maxlen, params))
        return false;

    // Your plugin initialization here...
    return true;
}
```

`CS2Kit::Initialize()` performs the following internally:

1. Sets up logging (built-in `ConsoleLogger` by default, or your custom `ILogger`)
2. Resolves all required SDK interfaces via `ISmmAPI::VInterfaceMatch()`
3. Sets the HL2SDK `g_pCVar` global
4. Loads engine signatures and offsets from built-in gamedata
5. Initializes all subsystems (schema, entities, events, menus, ConVars)

## Game Frame Hook

The scheduler and menu system require a per-tick callback. Hook `IServerGameDLL::GameFrame`:

```cpp
void MyPlugin::GameFrame(bool simulating, bool firstTick, bool lastTick)
{
    CS2Kit::OnGameFrame();
}
```

## Player Disconnect Hook

Clean up menu state when players disconnect:

```cpp
void MyPlugin::ClientDisconnect(CPlayerSlot slot, ...)
{
    CS2Kit::OnPlayerDisconnect(slot.Get());
}
```

## Shutdown

```cpp
bool MyPlugin::Unload(char* error, size_t maxlen)
{
    CS2Kit::Shutdown();
    return true;
}
```

## Next Steps

- @ref commands_guide — Register chat commands
- @ref menus_guide — Build interactive menus
- @ref sdk_guide — Access entities, schema fields, and engine services
