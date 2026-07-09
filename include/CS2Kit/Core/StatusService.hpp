#pragma once

#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace CS2Kit::Core
{

/**
 * @brief Aggregates named status sections for diagnostics commands.
 *
 * CS2Kit registers its own sections (build, load, gamedata, uptime) during load;
 * plugins add theirs in OnLoad and expose the report via a @ref Sdk::ServerCommand.
 * Providers run on demand, in registration order. Keep JSON output compact (counts
 * and names, not full lists) - RCON's console capture can truncate large responses.
 */
class StatusService
{
public:
    using Provider = std::function<nlohmann::json()>;

    /** @brief Add a section, replacing any existing one with the same name. */
    void RegisterSection(std::string name, Provider provider);

    /** @brief One object with a key per section: `{ "build": {...}, "load": {...}, ... }`. */
    nlohmann::json BuildJson() const;

    /** @brief Human-readable multi-line rendering of the same sections. */
    std::string BuildText() const;

private:
    std::vector<std::pair<std::string, Provider>> _sections;
};

}  // namespace CS2Kit::Core
