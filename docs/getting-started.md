# Getting Started {#getting_started}

[TOC]

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

The simplest way to start is to derive from @ref CS2Kit::Core::MetamodPluginBase. It runs
`CS2Kit::Initialize()`/`Shutdown()`, owns the four standard SourceHook hooks (GameFrame,
client connect/disconnect, chat dispatch), drives the `PlayerManager` lifecycle, and runs a
LIFO teardown stack on unload. You implement metadata plus your own logic:

```cpp
#include <CS2Kit/Core/MetamodPluginBase.hpp>

class MyPlugin : public CS2Kit::Core::MetamodPluginBase
{
protected:
    CS2Kit::Core::PluginInfo Info() const override
    {
        return { .Name = "My Plugin", .Author = "me", .Version = "1.0.0", .LogTag = "MINE" };
    }

    bool OnLoad(bool late) override
    {
        Defer([] { MySystem::Shutdown(); });  // cleanup runs automatically on unload / failed load
        return MySystem::Init();              // return false to reject the load
    }
};

MyPlugin g_MyPlugin;
PLUGIN_EXPOSE(MyPlugin, g_MyPlugin);  // in your .cpp
// PLUGIN_GLOBALVARS();               // in your plugin header
```

The base calls `CS2Kit::Initialize()` for you — it sets up logging, resolves the SDK interfaces, loads built-in gamedata, and initializes every subsystem (schema, entities, events, menus, ConVars). See the @ref plugin_guide for the full set of callbacks (player connect/disconnect, chat, custom hooks).

## Manual initialization (advanced)

If you cannot derive from `MetamodPluginBase`, call the lifecycle entry points yourself from your
own `ISmmPlugin`: `CS2Kit::Initialize()` in `Load()` (after `PLUGIN_SAVEVARS()`),
`CS2Kit::OnGameFrame()` from your `GameFrame` hook, `CS2Kit::OnPlayerDisconnect(slot.Get())` from
`ClientDisconnect`, and `CS2Kit::Shutdown()` in `Unload()`. You are then responsible for registering
hooks and calling `PlayerManager::AddPlayer`/`RemovePlayer`.

## Next Steps

- @ref plugin_guide — Build a plugin with MetamodPluginBase
- @ref commands_guide — Register chat commands
- @ref menus_guide — Build interactive menus
- @ref players_guide — Track connected players
- @ref sdk_guide — Access entities, schema fields, and engine services
