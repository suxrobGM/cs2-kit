#pragma once

#include <CS2Kit/Players/Player.hpp>
#include <CS2Kit/Utils/ChatColors.hpp>
#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace CS2Kit::Utils::Chat
{

/**
 * Send a colored chat line to the player at `slot`.
 * If `message` doesn't begin with a CS2 color escape (byte < 0x10), the default
 * color is prepended so the line renders cleanly.
 */
void Print(int slot, std::string_view message);

/**
 * Broadcast a colored chat line to all currently connected human players.
 * Bots and disconnected slots are skipped.
 */
void PrintAll(std::string_view message);

/**
 * Broadcast a colored chat line to every player matched by `filter`.
 * `filter` is invoked once per connected human; return `true` to deliver.
 */
void PrintFiltered(std::string_view message, const std::function<bool(const Players::Player*)>& filter);

/** Layout parameters for "{prefix} {actor} {phrase}" action lines; the caller supplies the
 *  prefix text (e.g. "[ADMIN]") and may recolor each segment. */
struct AdminLineStyle
{
    std::string Prefix;
    std::string_view PrefixColor = ChatColors::Green;
    std::string_view NameColor = ChatColors::Default;
    std::string_view PhraseColor = ChatColors::Olive;
};

/** "{prefix} {actor} {phrase}", e.g. "[ADMIN] Bob went stealth". */
std::string FormatAdminLine(const AdminLineStyle& style, std::string_view actorName, std::string_view phrase);

/** Single-target variant: "{prefix} {actor} {phrase} {target}", e.g. "[ADMIN] Bob slapped Alice". */
std::string FormatAdminLine(const AdminLineStyle& style, std::string_view actorName, std::string_view phrase,
                            std::string_view targetName);

/** Token variant for multi-target phrases, e.g. "swapped {a} and {b}": each mapped name is
 *  substituted into `phraseTemplate` wrapped in the name color. */
std::string FormatAdminLine(const AdminLineStyle& style, std::string_view actorName, std::string_view phraseTemplate,
                            const std::map<std::string, std::string>& nameTokens);

}  // namespace CS2Kit::Utils::Chat
