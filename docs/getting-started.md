# Getting Started {#getting_started}

[TOC]

## Prerequisites

- C++23 compiler
- HL2SDK CS2 submodule
- Metamod:Source 2.0 submodule
- CMake 4.3.4+
- Conan 2.29.1+
- Ninja

## Adding CS2Kit To Your Project

Add CS2Kit as a submodule:

```sh
git submodule add https://github.com/suxrobgm/cs2-kit.git vendor/cs2-kit
```

Then consume the CMake target:

```cmake
add_subdirectory(vendor/cs2-kit)
target_link_libraries(my-plugin PRIVATE CS2Kit::CS2Kit)
```

CS2Kit keeps HL2SDK and Metamod as source-tree submodules. Normal third-party
libraries come from Conan.

## Initialization

The simplest way to start is to derive from
`CS2Kit::MetamodPluginBase`. It runs `CS2Kit::Initialize()` and
`CS2Kit::Shutdown()`, owns the standard SourceHook hooks, drives the
`PlayerManager` lifecycle, and runs a LIFO teardown stack on unload.

> **Short names.** Public types are reachable as `CS2Kit::Type` (e.g.
> `CS2Kit::MetamodPluginBase`, `CS2Kit::PlayerController`, `CS2Kit::MenuBuilder`,
> `CS2Kit::Engine()`) by including `<CS2Kit/Api.hpp>`, which hoists the library's
> vocabulary out of its internal module namespaces (`Sdk`, `Menu`, `Core`, …).
> The fully-qualified `CS2Kit::Module::Type` spelling keeps working if you prefer
> it. Examples below use the short form.

```cpp
#include <CS2Kit/Api.hpp>

class MyPlugin : public CS2Kit::MetamodPluginBase
{
protected:
    CS2Kit::PluginInfo Info() const override
    {
        return { .Name = "My Plugin", .Author = "me", .Version = "1.0.0", .LogTag = "MINE" };
    }

    bool OnLoad(bool late) override
    {
        Defer([] { MySystem::Shutdown(); });
        return MySystem::Init();
    }
};

MyPlugin g_MyPlugin;
PLUGIN_EXPOSE(MyPlugin, g_MyPlugin);
```

## Manual Initialization

If you cannot derive from `MetamodPluginBase`, call the lifecycle entry points
yourself from your own `ISmmPlugin`: `CS2Kit::Initialize()` in `Load()`,
`CS2Kit::OnGameFrame()` from your frame hook, `CS2Kit::OnPlayerDisconnect()`
from disconnect handling, and `CS2Kit::Shutdown()` in `Unload()`.

## Next Steps

- @ref plugin_guide
- @ref commands_guide
- @ref menus_guide
- @ref players_guide
- @ref sdk_guide
