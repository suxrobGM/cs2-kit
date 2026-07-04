# Getting Started {#getting_started}

[TOC]

## Prerequisites

- C++23 compiler (MSVC 2022+, or the Steam Runtime toolchain for Linux)
- CMake 4.3.4+, Conan 2.29.1+, and Ninja - all three are pinned in `pyproject.toml`, so `uv sync` installs them into the project environment (or install globally via pip/pipx)

The HL2SDK and Metamod:Source come with the kit as submodules - you don't install them separately.

## Add the kit to your repo

```sh
git submodule add https://github.com/suxrobgm/cs2-kit.git vendor/cs2-kit
git submodule update --init --recursive
```

```cmake
add_subdirectory(vendor/cs2-kit)
target_link_libraries(my-plugin PRIVATE CS2Kit::CS2Kit)
```

Normal third-party libraries come from Conan; the Conan profiles under `vendor/cs2-kit/conan/profiles/` are canonical, so consuming repos reuse them instead of keeping copies.

## Scaffold a plugin

The kit ships a generator that stamps a working plugin - it builds, loads, and answers `!ping` before you write a line of code:

```sh
uv run poe new-plugin my-plugin        # from your repo's root
```

You get `plugins/my-plugin/` with a `PluginBase` skeleton, a self-registering example command, a `settings.jsonc` mapped by @ref CS2Kit::Core::JsonConfig, and translations. The root `CMakeLists.txt` gains its `add_subdirectory` line automatically. Expose the task in your own `pyproject.toml` as:

```toml
[tool.poe.tasks]
new-plugin = "python vendor/cs2-kit/scripts/new_plugin.py"
```

## The skeleton, by hand

If you'd rather see what the generator writes: derive from @ref CS2Kit::Core::PluginBase with your manager struct. The base runs `CS2Kit::Initialize()`/`Shutdown()`, owns the standard SourceHook hooks and the player lifecycle, constructs your managers once the kit services are live, and tears everything down LIFO on unload.

> **Short names.** Including `<CS2Kit/Api.hpp>` hoists the public vocabulary to `CS2Kit::Type`
> (`CS2Kit::PluginBase`, `CS2Kit::CommandSpec`, `CS2Kit::Engine()`, ...), so you don't spell out
> the internal module namespaces. The fully-qualified `CS2Kit::Module::Type` forms keep working.
> Examples in these guides use the short form.

```cpp
#include <CS2Kit/Api.hpp>

struct Managers
{
    ConfigManager Config;   // your managers, in dependency order
};
Managers& App();            // free accessor, forwarded below

class MyPlugin : public CS2Kit::PluginBase<Managers>
{
protected:
    CS2Kit::PluginInfo Info() const override
    {
        return { .Name = "My Plugin", .Author = "me", .Version = "1.0.0", .LogTag = "MINE" };
    }

    bool OnLoad(bool late) override
    {
        return App().Config.Load("addons/my-plugin/configs/settings.jsonc");
    }
};

MyPlugin g_MyPlugin;
PLUGIN_EXPOSE(MyPlugin, g_MyPlugin);

Managers& App() { return MyPlugin::App(); }
```

## Manual initialization

If you can't derive from the base, call the entry points from your own `ISmmPlugin`: `CS2Kit::Initialize()` in `Load()`, `CS2Kit::OnGameFrame()` from your frame hook, `CS2Kit::OnPlayerDisconnect()` from disconnect handling, and `CS2Kit::Shutdown()` in `Unload()`.

## Next steps

- @ref plugin_guide - what the base owns and what you override
- @ref commands_guide - add real commands
- @ref config_guide - grow the settings file
- @ref menus_guide - menus and wizards
