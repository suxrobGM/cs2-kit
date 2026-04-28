#pragma once

#include <CS2Kit/Players/Player.hpp>
#include <functional>
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

}  // namespace CS2Kit::Utils::Chat
