# CS2Kit

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://suxrobgm.github.io/cs2-kit/)

A C++23 toolkit for building Counter-Strike 2 server plugins on Metamod:Source 2.0.

CS2Kit handles the boilerplate of a CS2 plugin - engine setup, hooks, player tracking, menus, chat, and config - so you can focus on your plugin's actual features.

> **Work in progress.** The API is still evolving and may change between versions.

## What you get

- **Plugin base** - subclass one class and your plugin lifecycle, hooks, and player tracking are wired up for you.
- **Chat commands** - register `!commands` with aliases, argument checks, and permission gating.
- **Menus** - WASD-navigated in-game menus with buttons, toggles, sliders, choices, submenus, and ready-made presets (player picker, duration picker, confirm dialog).
- **Players** - fast lookup of connected players by slot or SteamID, target-token resolution (`@all`, `#slot`, SteamID, name), and a policy-injected action dispatcher.
- **Engine SDK** - friendly wrappers over entities, ConVars, game events, messages, common pawn operations (teleport, freeze, godmode, slap), and persistent center-HTML panels.
- **HTTP** - async requests with game-thread completions, plus config-driven JSON endpoint helpers.
- **PostgreSQL** (optional) - connection + prepared-statement client and a migration runner, gated behind `CS2KIT_ENABLE_POSTGRES`.
- **Utilities** - JSON config, SteamID conversions, colored chat, token-substituting translations, per-slot effect registry, and timers.

## Quick start

Add CS2Kit to your plugin as a submodule:

```sh
git submodule add https://github.com/suxrobgm/cs2-kit.git vendor/cs2-kit
```

Then derive your plugin from `MetamodPluginBase` and implement what you need:

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
        Defer([] { MySystem::Shutdown(); });  // cleanup runs automatically on unload
        return MySystem::Init();              // return false to abort the load
    }

    void OnPlayerConnect(CS2Kit::Players::Player* player) override { /* ... */ }
};

MyPlugin g_MyPlugin;
PLUGIN_EXPOSE(MyPlugin, g_MyPlugin);
```

That's a working plugin - the base sets up the engine, registers the standard hooks, and tracks players. The [Getting Started guide](https://suxrobgm.github.io/cs2-kit/) walks through the build setup and the rest of the callbacks.

## Documentation

Full guides and API reference: **[suxrobgm.github.io/cs2-kit](https://suxrobgm.github.io/cs2-kit/)**

- **Getting Started** - install, build setup, and your first plugin
- **Plugin Base** - lifecycle, callbacks, and custom hooks
- **Commands, Menus, Players, SDK, Chat, Database, HTTP** - per-feature guides

## Contributing

Contributions are welcome. Since the API is still evolving, it's worth opening an issue to discuss larger changes first.

## License

[MIT](LICENSE)
