# Plugin Base {#plugin_guide}

[TOC]

@ref CS2Kit::Core::MetamodPluginBase removes the boilerplate every Metamod:Source plugin
otherwise re-types: the ISmmPlugin metadata getters, the `Load`/`Unload` skeleton, the standard
SourceHook hooks, and the `PlayerManager` add/remove lifecycle. You subclass it, return your
metadata, and implement only the callbacks you care about.

## What the base owns

- **Metadata getters** — all eight `Get*()` methods are answered from @ref CS2Kit::Core::PluginInfo.
- **Lifecycle** — `Load()` runs `PLUGIN_SAVEVARS()`, `CS2Kit::Initialize()`, registers the standard
  hooks, then calls your `OnLoad()`. If `OnLoad()` returns `false`, the teardown stack runs and
  `CS2Kit::Shutdown()` is called before the load is rejected — so a failed init never leaks
  initialized subsystems.
- **Standard hooks** — `GameFrame` → `CS2Kit::OnGameFrame()`; client connect → `PlayerManager::AddPlayer`
  then `OnPlayerConnect`; client disconnect → `OnPlayerDisconnect` then `CS2Kit::OnPlayerDisconnect`
  then `PlayerManager::RemovePlayer`; `say`/`say_team` → parsed and routed to `OnPlayerChat`.
- **Teardown stack** — `Defer()` registers cleanups that run in reverse (LIFO) on unload or failed load.

## Required overrides

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
        Defer([] { Database::Close(); });   // teardown registered next to its setup
        return Database::Open();            // false rejects the load
    }
};
```

## Optional overrides

| Override | When it fires | Notes |
|----------|---------------|-------|
| `OnUnload()` | On unload, before the teardown stack | Prefer `Defer()` for most cleanup |
| `OnPlayerConnect(Player*)` | After the player is added to `PlayerManager` | Non-null in the normal flow |
| `OnPlayerDisconnect(Player*)` | Before the player is removed | May be null — guard it |
| `OnPlayerChat(Player*, string_view, bool team)` | On `say` / `say_team` | Return `true` to supercede (suppress the original chat) |
| `OnRegisterHooks()` | Once during load, after the standard hooks | Add custom SourceHook hooks here |

## Custom hooks

The base owns the four common hooks. For anything else, register it in `OnRegisterHooks()` and pair
it with a `Defer()` for removal. Because `SH_DECL_HOOK` generates types at file scope, the
declaration and the add/remove calls live in your own translation unit:

```cpp
SH_DECL_HOOK3(IVEngineServer2, SetClientListening, SH_NOATTRIB, 0, bool, CPlayerSlot, CPlayerSlot, bool);

void MyPlugin::OnRegisterHooks()
{
    auto& gi = CS2Kit::Core::Kit().Interfaces;
    SH_ADD_HOOK(IVEngineServer2, SetClientListening, gi.Engine,
                SH_MEMBER(this, &MyPlugin::Hook_SetClientListening), false);

    Defer([this] {
        auto& g = CS2Kit::Core::Kit().Interfaces;
        SH_REMOVE_HOOK(IVEngineServer2, SetClientListening, g.Engine,
                       SH_MEMBER(this, &MyPlugin::Hook_SetClientListening), false);
    });
}
```

## Required macros

Two Metamod macros stay in the consuming plugin (they define the per-plugin SourceHook globals and
the exported entry point the base links against):

```cpp
// In your plugin header:
PLUGIN_GLOBALVARS();

// In your plugin .cpp:
MyPlugin g_MyPlugin;
PLUGIN_EXPOSE(MyPlugin, g_MyPlugin);
```

## Configuration

Pair the base with @ref CS2Kit::Utils::Json for settings. Define a struct that mirrors your JSON
(member names must match the keys) and map it once with the nlohmann macro — missing keys keep their
defaults, so only a malformed file or a wrong-typed value fails the load:

```cpp
struct Settings {
    std::string host = "localhost";
    int port = 5432;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Settings, host, port)

bool OnLoad(bool late) override
{
    auto cfg = CS2Kit::Utils::Json::TryDeserializeFile<Settings>("addons/my-plugin/config.json");
    if (!cfg)
        return false;  // missing/unparseable file, or a wrong-typed value
    Connect(cfg->host, cfg->port);
    return true;
}
```
