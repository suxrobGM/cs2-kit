"""Stamp a new plugin skeleton under plugins/<name>/ in the consuming repo.

Usage (from the consuming repo's root): uv run poe new-plugin <name>

<name> is kebab-case (e.g. "fun-votes"). The generated plugin builds as-is: it loads
settings.jsonc, installs a permissive policy, and answers !ping. The repo's root
CMakeLists.txt gets its add_subdirectory() line inserted automatically.

The script lives in cs2-kit but targets the current working directory, so any repo
that vendors the kit can expose it as a task, e.g.:

    new-plugin = "python vendor/cs2-kit/scripts/new_plugin.py"
"""

import re
import sys
from pathlib import Path

REPO_ROOT = Path.cwd()

NAME_RE = re.compile(r"^[a-z][a-z0-9]*(-[a-z0-9]+)*$")


def pascal_case(name: str) -> str:
    return "".join(part.capitalize() for part in name.split("-"))


def render_templates(name: str) -> dict[str, str]:
    pascal = pascal_case(name)
    ns = pascal
    klass = f"{pascal}Plugin"
    title = " ".join(part.capitalize() for part in name.split("-"))
    tag = pascal.upper()[:12]

    cmakelists = f"""# Sources default to a CONFIGURE_DEPENDS glob of src/*.cpp in cs2_add_plugin.
cs2_add_plugin({name}
    LIBRARIES
        nlohmann_json::nlohmann_json
)
"""

    plugin_hpp = f"""#pragma once

#include "Managers.hpp"

#include <CS2Kit/Api.hpp>

/**
 * {title} plugin entry point. CS2Kit::PluginBase owns the Metamod lifecycle, standard
 * hooks, player tracking, and the Managers container; this class adds plugin metadata
 * and subsystem wiring. Reach the managers via {ns}::App().
 */
class {klass} : public CS2Kit::PluginBase<{ns}::Managers>
{{
protected:
    CS2Kit::PluginInfo Info() const override;
    bool OnLoad(bool late) override;
    bool OnPlayerChat(CS2Kit::Player* player, std::string_view message, bool teamChat) override;
}};

extern {klass} g_{klass};

PLUGIN_GLOBALVARS();
"""

    plugin_cpp = f"""#include "Plugin.hpp"

#include "Config.hpp"
#include "Managers.hpp"

#include <CS2Kit/Api.hpp>
#include <CS2Kit/Commands/CommandManager.hpp>
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <CS2Kit/Utils/Translations.hpp>
#include <string>

using CS2Kit::Core::Engine;
namespace Log = CS2Kit::Utils::Log;

{klass} g_{klass};
PLUGIN_EXPOSE({klass}, g_{klass});

namespace {ns}
{{
Managers& App()
{{
    return {klass}::App();
}}
}}  // namespace {ns}

CS2Kit::PluginInfo {klass}::Info() const
{{
    return CS2Kit::PluginInfo{{
        .Name = "{title}",
        .Author = "TODO",
        .Description = "TODO",
        .Url = "",
        .License = "MIT",
        .Version = "0.1.0",
        .LogTag = "{tag}",
    }};
}}

bool {klass}::OnLoad(bool late)
{{
    if (!{ns}::App().Config.Load("addons/{name}/configs/settings.jsonc"))
        return false;

    Engine().Translations.SetLanguage({ns}::App().Config.Get().plugin.locale);
    Engine().Translations.Load("addons/{name}/configs/translations");

    // The kit consults this policy for command permissions, targeting, and replies.
    // Replace the permissive defaults once the plugin has real permission data.
    Engine().Policy = {{
        .HasPermission = [](int64_t, const std::string&) {{ return true; }},
        .CanTarget = [](CS2Kit::Player&, CS2Kit::Player&) {{ return true; }},
        .Reply = [](int slot, std::string_view msg) {{ Engine().Messages.Reply(slot, msg); }},
    }};

    // Commands self-registered at their definition sites (see Commands.cpp); ingest once.
    Engine().Commands.RegisterAll(CS2Kit::Registry<CS2Kit::CommandSpec>::Items());

    Log::Info("Loaded v{{}}.", Info().Version);
    return true;
}}

bool {klass}::OnPlayerChat(CS2Kit::Player* player, std::string_view message, bool /*teamChat*/)
{{
    if (!player || message.empty())
        return false;
    if (message.front() != '!' && message.front() != '.')
        return false;

    // Swallow the line only when it matched a registered command; unknown "!words"
    // fall through to normal chat.
    return Engine().Commands.HandleChatMessage(player, std::string(message));
}}
"""

    managers_hpp = f"""#pragma once

#include "Config.hpp"

namespace {ns}
{{

struct Managers;

/** The plugin's live managers. Valid only between OnLoad and unload. */
Managers& App();

/**
 * Plugin-owned managers, constructed by PluginBase after the kit services are live and
 * destroyed on unload - state cannot leak across `meta reload`. Declared in dependency
 * order; destroyed in reverse.
 */
struct Managers
{{
    ConfigManager Config;
}};

}}  // namespace {ns}
"""

    config_hpp = f"""#pragma once

#include <CS2Kit/Api.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace {ns}
{{

/** "plugin" section of settings.jsonc. */
struct PluginSettings
{{
    std::string logLevel = "info";
    std::string locale = "en";
}};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PluginSettings, logLevel, locale)

/** Root of settings.jsonc. Add a struct + a member here for each new section. */
struct Settings
{{
    PluginSettings plugin;
}};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Settings, plugin)

/** Subclass CS2Kit::JsonConfig instead once you need post-load validation or accessors. */
using ConfigManager = CS2Kit::JsonConfig<Settings>;

}}  // namespace {ns}
"""

    commands_cpp = """#include "Managers.hpp"

#include <CS2Kit/Api.hpp>

using namespace CS2Kit::Commands;

// Commands self-register into the Registry at their definition site; Plugin.cpp ingests
// them once in OnLoad. Add more specs here (or in new .cpp files) and they are picked up
// automatically - no central registration list to maintain.
static const bool _pingRegistered = CS2Kit::Registry<CS2Kit::CommandSpec>::Add({
    .Name = "ping",
    .Description = "Check that the plugin is alive.",
    .Usage = "!ping",
    .Handler = [](CommandContext& c) { return c.Ok("cmd.pong"); },
});
"""

    settings_jsonc = """{
  // Plugin-wide options.
  "plugin": {
    "logLevel": "info",
    "locale": "en"
  }
}
"""

    en_json = """{
  "cmd.pong": "Pong!"
}
"""

    return {
        "CMakeLists.txt": cmakelists,
        "src/Plugin.hpp": plugin_hpp,
        "src/Plugin.cpp": plugin_cpp,
        "src/Managers.hpp": managers_hpp,
        "src/Config.hpp": config_hpp,
        "src/Commands.cpp": commands_cpp,
        "configs/settings.jsonc": settings_jsonc,
        "configs/translations/en.json": en_json,
    }


def insert_subdirectory(root_cmake: Path, name: str) -> bool:
    """Add add_subdirectory(plugins/<name>) after the last plugin add_subdirectory."""
    line = f"add_subdirectory(plugins/{name})"
    text = root_cmake.read_text(encoding="utf-8")
    if line in text:
        return False

    lines = text.splitlines(keepends=True)
    last = max(
        (i for i, ln in enumerate(lines) if ln.strip().startswith("add_subdirectory(plugins/")),
        default=len(lines) - 1,
    )
    lines.insert(last + 1, line + "\n")
    root_cmake.write_text("".join(lines), encoding="utf-8", newline="\n")
    return True


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__.strip())
        return 2

    name = sys.argv[1]
    if not NAME_RE.match(name):
        print(f"error: '{name}' is not kebab-case (expected e.g. 'fun-votes').")
        return 2

    if not (REPO_ROOT / "CMakeLists.txt").is_file():
        print(f"error: no CMakeLists.txt in {REPO_ROOT}; run from your repo's root.")
        return 1

    plugin_dir = REPO_ROOT / "plugins" / name
    if plugin_dir.exists():
        print(f"error: {plugin_dir} already exists; refusing to overwrite.")
        return 1

    for rel_path, content in render_templates(name).items():
        path = plugin_dir / rel_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8", newline="\n")
        print(f"  created plugins/{name}/{rel_path}")

    if insert_subdirectory(REPO_ROOT / "CMakeLists.txt", name):
        print(f"  registered add_subdirectory(plugins/{name}) in CMakeLists.txt")

    print("\nDone. Build it with: uv run poe build")
    return 0


if __name__ == "__main__":
    sys.exit(main())
