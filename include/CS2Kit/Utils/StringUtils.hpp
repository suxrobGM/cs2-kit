#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace CS2Kit::Utils
{

/**
 * Parse a human duration string into seconds. Accepts a bare integer (seconds) or an
 * integer with a unit suffix: `s` seconds, `m` minutes, `h` hours, `d` days. The literals
 * `0`, `perm`, and `permanent` mean "permanent" (returns 0). Surrounding whitespace is ignored.
 * Returns -1 on parse failure, 0 for permanent, otherwise the duration in seconds.
 */
int ParseDuration(std::string_view text);

/** @brief Collection of static string manipulation utilities. */
class StringUtils
{
public:
    static std::string ToLower(const std::string& str);
    static std::string ToUpper(const std::string& str);
    static std::string Trim(const std::string& str);
    static std::string TrimLeft(const std::string& str);
    static std::string TrimRight(const std::string& str);
    static std::vector<std::string> Split(const std::string& str, char delimiter);
    static std::string Join(const std::vector<std::string>& parts, const std::string& delimiter);
    static bool StartsWith(const std::string& str, const std::string& prefix);
    static bool EndsWith(const std::string& str, const std::string& suffix);
    static bool ContainsIgnoreCase(const std::string& str, const std::string& substr);
    static std::string ReplaceAll(const std::string& str, const std::string& from, const std::string& to);

    /** Replace each `{key}` occurrence in @p text with its mapped value. */
    static std::string SubstituteTokens(std::string text, const std::map<std::string, std::string>& tokens);

    static bool IsNumeric(const std::string& str);

    /** How a command target string (e.g. "@all", "#3", "player name", SteamID) was parsed. */
    enum class TargetType
    {
        All,
        Me,
        Index,
        Name,
        SteamId
    };

    /** Result of parsing a target string into type + value. */
    struct TargetInfo
    {
        TargetType Type;
        std::string Value;
    };

    static TargetInfo ParseTarget(const std::string& target);
};

}  // namespace CS2Kit::Utils
