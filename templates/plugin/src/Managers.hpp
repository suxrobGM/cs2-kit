#pragma once

#include "Config.hpp"

namespace $ns
{

struct Managers;

/** The plugin's live managers. Valid only between OnLoad and unload. */
Managers& App();

/**
 * Plugin-owned managers, constructed by PluginBase after the kit services are live and
 * destroyed on unload - state cannot leak across `meta reload`. Declared in dependency
 * order; destroyed in reverse.
 */
struct Managers
{
    ConfigManager Config;
};

}  // namespace $ns
