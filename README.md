# CS2Kit

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://suxrobgm.github.io/cs2-kit/)

A C++23 toolkit for building Counter-Strike 2 server plugins on Metamod:Source 2.0.

You describe your plugin's behavior as data - commands, menu rows, effects, database rows - and CS2Kit owns the machinery around it: engine setup, hooks, player tracking, permission gating, message transport, and teardown.

> **Work in progress.** The API is still evolving and may change between versions.

## What you get

- **Plugin base** - subclass `PluginBase<Managers>` and the Metamod lifecycle, standard hooks, player tracking, and your own manager container are wired up; cleanup is a LIFO `Defer` stack.
- **Declarative commands** - a `CommandSpec` self-registers where it's defined; targets, durations, and SteamIDs are resolved and validated before your handler runs, with localized error replies.
- **Target selectors** - `@all`, `@me`, `@t`, `@ct`, `@dead`, `@random`, `#slot`, SteamIDs, and name fragments, with immunity applied through your injected policy.
- **Menus** - WASD-navigated in-game menus: typed rows, policy-aware context rows for admin/target actions, ready-made pickers, and the `Flow` wizard for multi-step "pick duration → pick reason → confirm" chains.
- **One policy hook** - permissions, immunity, replies, and broadcasts are injected once via `Engine().Policy`; the kit ships no admin model of its own.
- **Messages** - one service for chat, center print, center-HTML, and alerts, with per-player translations (`ReplyKey`) and chat colors.
- **Typed game events** - `Listen<PlayerDeath>(...)` instead of string names and `GetInt` calls; the raw overload stays for unmodeled events.
- **PostgreSQL** (optional) - async-first client (worker thread owns the connection, completions on the game thread), column-table row mapping that generates the INSERT/SELECT/parse code, and a migration runner. Gated behind `CS2KIT_ENABLE_POSTGRES`.
- **HTTP** - async requests with game-thread completions, plus config-driven JSON endpoint helpers.
- **Scaffold generator** - `scripts/new_plugin.py` stamps a buildable plugin (skeleton, settings, translations, example command) into your repo from the `templates/plugin/` tree.

## Quick start

Add CS2Kit to your repo as a submodule and link the target:

```sh
git submodule add https://github.com/suxrobgm/cs2-kit.git vendor/cs2-kit
git submodule update --init --recursive
```

```cmake
add_subdirectory(vendor/cs2-kit)
target_link_libraries(my-plugin PRIVATE CS2Kit::CS2Kit)
```

Then generate a plugin - it compiles, loads, and answers `!ping` out of the box:

```sh
uv run poe new-plugin my-plugin
```

Or write the skeleton yourself:

```cpp
#include <CS2Kit/Api.hpp>

struct Managers { ConfigManager Config; };
Managers& App();

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

A command is one self-registering aggregate - no dispatcher wiring, no arg parsing:

```cpp
static const bool _registered = CS2Kit::Registry<CS2Kit::CommandSpec>::Add({
    .Name = "slap",
    .Usage = "!slap <target>",
    .Permission = "s",
    .Args = {Target()},
    .Handler = [](CommandContext& c) {
        CS2Kit::PawnOps::Slap(c.Target->Controller());
        return c.Ok("cmd.slapped", {{"name", c.Target->GetName()}});
    },
});
```

The [Getting Started guide](https://suxrobgm.github.io/cs2-kit/) walks through the build setup and the rest.

## Documentation

Full guides and API reference: **[suxrobgm.github.io/cs2-kit](https://suxrobgm.github.io/cs2-kit/)**

- **Getting Started** - install, scaffold, build
- **Architecture** - services, policy, lifetimes
- **Plugin Base, Configuration, Commands, Menus, Players, Messages, SDK, Database, HTTP** - per-feature guides

## Contributing

Contributions are welcome. Since the API is still evolving, it's worth opening an issue to discuss larger changes first.

## License

[MIT](LICENSE)
