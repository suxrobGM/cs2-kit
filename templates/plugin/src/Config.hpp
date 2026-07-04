#pragma once

#include <CS2Kit/Api.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace $ns
{

/** "plugin" section of settings.jsonc. */
struct PluginSettings
{
    std::string logLevel = "info";
    std::string locale = "en";
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PluginSettings, logLevel, locale)

/** Root of settings.jsonc. Add a struct + a member here for each new section. */
struct Settings
{
    PluginSettings plugin;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Settings, plugin)

/** Subclass CS2Kit::JsonConfig instead once you need post-load validation or accessors. */
using ConfigManager = CS2Kit::JsonConfig<Settings>;

}  // namespace $ns
