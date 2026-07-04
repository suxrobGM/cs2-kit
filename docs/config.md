# Configuration {#config_guide}

[TOC]

Settings are a struct that mirrors your JSON file, deserialized in one call. Declare each section as a struct with defaults, map it with nlohmann's non-intrusive macro, and hold the root in a @ref CS2Kit::Core::JsonConfig.

## Declaring settings

```cpp
#include <CS2Kit/Api.hpp>
#include <nlohmann/json.hpp>

struct PluginSettings
{
    std::string logLevel = "info";
    std::string locale = "en";
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PluginSettings, logLevel, locale)

struct Settings
{
    PluginSettings plugin;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Settings, plugin)

using ConfigManager = CS2Kit::JsonConfig<Settings>;
```

Member names must match the JSON keys. The `_WITH_DEFAULT` macro means a missing key keeps the member's default - only a missing file, a parse error, or a wrong-typed value fails the load. JSONC comments are tolerated.

```cpp
bool MyPlugin::OnLoad(bool late)
{
    if (!App().Config.Load("addons/my-plugin/configs/settings.jsonc"))
        return false;   // logged already; reject the load

    const auto& s = App().Config.Get();
    Engine().Translations.SetLanguage(s.plugin.locale);
    return true;
}
```

## Post-load validation

When raw settings need parsing or clamping (duration strings, tag sanitizing, dropping invalid list entries), subclass `JsonConfig` and resolve once after `Load`:

```cpp
class ConfigManager : public CS2Kit::JsonConfig<Settings>
{
public:
    bool LoadSettings(const std::string& path)
    {
        if (!Load(path))
            return false;
        Resolve();
        return true;
    }

    const std::vector<int>& GetMenuDurations() const { return _menuDurationSecs; }

private:
    void Resolve();
    std::vector<int> _menuDurationSecs;
};
```

`Utils/Validation.hpp` (`CS2Kit::Validation`) has the common resolution helpers:

```cpp
using namespace CS2Kit;

void ConfigManager::Resolve()
{
    auto& s = Mutable();

    // Clamp + fall back with a logged warning; "server.tag" names the field in the log line.
    Validation::NormalizeTag(s.server.tag, 32, "default", "server.tag");

    // Drop entries a predicate rejects, logging each removal.
    Validation::FilterValid(s.punishments.templates,
                            [](const auto& t) { return IsKnownType(t.type); }, "punishment template");

    // "5m"/"1h"/"perm" strings -> seconds (0 = permanent); invalid entries logged and skipped.
    _menuDurationSecs = Validation::ParseDurations(s.punishments.menuDurations, "menu duration");
}
```

## Kit types in your settings

`PostgresConfig` uses lowercase field names precisely so a JSON section maps onto it. The kit header stays nlohmann-free - define the mapper in your plugin, inside the `CS2Kit::Database` namespace so ADL finds it:

```cpp
namespace CS2Kit::Database
{
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PostgresConfig, host, port, database, username, password, sslMode)
}

struct Settings
{
    CS2Kit::PostgresConfig database;   // "database": { "host": ..., "port": ... }
    // ...
};
```

## Translations

Human-facing text lives in per-language JSON files (`translations/en.json`, `translations/ru.json`, ...), flat key → string with `{token}` placeholders:

```json
{
    "cmd.banSuccess": "Banned {name}.",
    "target.noMatch": "No player matched."
}
```

```cpp
Engine().Translations.Load("addons/my-plugin/configs/translations");
Engine().Translations.SetLanguage("en");                       // server default
Engine().Translations.SetPlayerLanguage(slot, "ru");           // per-player override

auto line = Engine().Translations.Get("cmd.banSuccess", slot, {{"name", targetName}});
```

Command results (`CommandContext::Ok`/`Fail`), `Flow` validation errors, and `MessageSystem::ReplyKey` all resolve through this service in the addressed player's language. The kit reserves a handful of keys for its own error replies - see @ref commands_guide.
