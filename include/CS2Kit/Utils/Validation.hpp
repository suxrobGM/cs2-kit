#pragma once

#include <CS2Kit/Utils/Log.hpp>
#include <CS2Kit/Utils/StringUtils.hpp>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace CS2Kit::Utils::Validation
{

/**
 * @brief Declarative helpers for post-load config validation: skip bad entries with a
 * warning instead of failing the whole load, and fall back to sane defaults.
 */

/** Reset @p value to @p fallback (warning names @p what) when it is blank or longer than @p maxLen. */
inline void NormalizeTag(std::string& value, std::size_t maxLen, std::string_view fallback, std::string_view what)
{
    if (!StringUtils::Trim(value).empty() && value.size() <= maxLen)
        return;

    Log::Warn("settings: {} is empty or longer than {} chars; using \"{}\"", what, maxLen, fallback);
    value = std::string(fallback);
}

/** Keep only items where @p validate(item, index) returns nullopt; each rejection logs its reason.
 *  @p validate: `std::optional<std::string>(const T&, std::size_t index)`. */
template <class T, class Fn>
void FilterValid(std::vector<T>& items, Fn&& validate, std::string_view what)
{
    std::vector<T> kept;
    kept.reserve(items.size());
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        if (auto reason = validate(items[i], i))
            Log::Warn("settings: skipping {}[{}]: {}", what, i, *reason);
        else
            kept.push_back(std::move(items[i]));
    }
    items = std::move(kept);
}

/** Parse @ref ParseDuration strings, dropping invalid entries with warnings. Falls back to
 *  parsing @p defaults when nothing valid remains (so the list exists in exactly one place). */
inline std::vector<int> ParseDurations(const std::vector<std::string>& entries,
                                       const std::vector<std::string>& defaults, std::string_view what)
{
    std::vector<int> result;
    for (std::size_t i = 0; i < entries.size(); ++i)
    {
        int seconds = ParseDuration(entries[i]);
        if (seconds < 0)
        {
            Log::Warn("settings: skipping {}[{}]: bad duration '{}'", what, i, entries[i]);
            continue;
        }
        result.push_back(seconds);
    }

    if (result.empty() && !defaults.empty())
    {
        Log::Warn("settings: {} has no valid entries; using built-in defaults", what);
        for (const auto& entry : defaults)
            result.push_back(ParseDuration(entry));
    }
    return result;
}

}  // namespace CS2Kit::Utils::Validation
