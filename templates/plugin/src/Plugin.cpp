#include "Plugin.hpp"

#include "Config.hpp"
#include "Managers.hpp"

#include <CS2Kit/Api.hpp>
#include <CS2Kit/BuildInfo.hpp>
#include <CS2Kit/Commands/CommandManager.hpp>
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <CS2Kit/Utils/Translations.hpp>
#include <string>

using CS2Kit::Core::Engine;
namespace Log = CS2Kit::Utils::Log;

$klass g_$klass;
PLUGIN_EXPOSE($klass, g_$klass);

namespace $ns
{
Managers& App()
{
    return $klass::App();
}
}  // namespace $ns

CS2Kit::PluginInfo $klass::Info() const
{
    return CS2Kit::PluginInfo{
        .Name = "$title",
        .Author = "TODO",
        .Description = "TODO",
        .Url = "",
        .License = "MIT",
        .Version = CS2Kit::BuildInfo::Version,
        .Date = CS2Kit::BuildInfo::BuildDate,
        .Commit = CS2Kit::BuildInfo::RepoCommit,
        .LogTag = "$tag",
    };
}

bool $klass::OnLoad(bool late)
{
    if (!$ns::App().Config.Load("addons/$name/configs/settings.jsonc"))
        return false;

    Engine().Translations.SetLanguage($ns::App().Config.Get().plugin.locale);
    Engine().Translations.Load("addons/$name/configs/translations");

    // The kit consults this policy for command permissions, targeting, and replies.
    // Replace the permissive defaults once the plugin has real permission data.
    Engine().Policy = {
        .HasPermission = [](int64_t, const std::string&) { return true; },
        .CanTarget = [](CS2Kit::Player&, CS2Kit::Player&) { return true; },
        .Reply = [](int slot, std::string_view msg) { Engine().Messages.Reply(slot, msg); },
    };

    // Commands self-registered at their definition sites (see Commands.cpp); ingest once.
    Engine().Commands.RegisterAll(CS2Kit::Registry<CS2Kit::CommandSpec>::Items());

    Log::Info("Loaded v{}.", Info().Version);
    return true;
}

bool $klass::OnPlayerChat(CS2Kit::Player* player, std::string_view message, bool /*teamChat*/)
{
    if (!player || message.empty())
        return false;
    if (message.front() != '!' && message.front() != '.')
        return false;

    // Swallow the line only when it matched a registered command; unknown "!words"
    // fall through to normal chat.
    return Engine().Commands.HandleChatMessage(player, std::string(message));
}
