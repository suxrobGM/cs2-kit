#pragma once

#include <string>
#include <vector>

namespace CS2Kit::Utils
{

/** Collection of static string manipulation utilities. */
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
