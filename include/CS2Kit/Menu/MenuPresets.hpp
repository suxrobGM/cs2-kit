#pragma once

#include <CS2Kit/Menu/Menu.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace CS2Kit::Menu
{

/**
 * @brief Reusable, content-agnostic menu builders.
 *
 * These presets take every human-facing string as a parameter so they carry no
 * localization of their own — the caller supplies already-translated text.
 */

/**
 * Build a paginated picker listing every connected player (from the kit PlayerManager).
 * Selecting a player invokes @p onPick(viewerSlot, targetSlot). @p isEnabled, when supplied,
 * decides per-row whether a target is selectable. If no players are connected, a single
 * disabled @p emptyLabel row is shown instead.
 */
std::shared_ptr<Menu> BuildPlayerPicker(int viewerSlot, const std::string& title,
                                        std::function<void(int viewerSlot, int targetSlot)> onPick,
                                        const std::string& emptyLabel = "",
                                        std::function<bool(int targetSlot)> isEnabled = {});

/**
 * Build a duration picker. Each @p presets entry is a (label, seconds) pair rendered as a row;
 * selecting it invokes @p onPick(viewerSlot, seconds). When @p customLabel is non-empty an
 * extra chat-input row is appended: it prompts with @p customPrompt and parses the typed text
 * via @ref CS2Kit::Utils::ParseDuration (rejecting/re-prompting on a negative result).
 */
std::shared_ptr<Menu> BuildDurationPicker(int viewerSlot, const std::string& title,
                                          const std::vector<std::pair<std::string, int>>& presets,
                                          std::function<void(int viewerSlot, int seconds)> onPick,
                                          const std::string& customLabel = "",
                                          const std::string& customPrompt = "", int maxInputLen = 32);

}  // namespace CS2Kit::Menu
