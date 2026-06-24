#include <CS2Kit/Utils/SteamId.hpp>
#include <CS2Kit/Utils/StringUtils.hpp>
#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>

namespace CS2Kit::Utils
{

int ParseDuration(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
        text.remove_prefix(1);
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
        text.remove_suffix(1);

    if (text.empty())
        return -1;
    if (text == "0" || text == "perm" || text == "permanent")
        return 0;

    int multiplier = 1;
    char suffix = text.back();
    if (!std::isdigit(static_cast<unsigned char>(suffix)))
    {
        switch (suffix)
        {
        case 's':
            multiplier = 1;
            break;
        case 'm':
            multiplier = 60;
            break;
        case 'h':
            multiplier = 3600;
            break;
        case 'd':
            multiplier = 86400;
            break;
        default:
            return -1;
        }
        text.remove_suffix(1);
    }

    int value = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size() || value < 0)
        return -1;

    return value * multiplier;
}

std::string StringUtils::ToLower(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string StringUtils::ToUpper(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string StringUtils::TrimLeft(const std::string& str)
{
    auto it = std::find_if(str.begin(), str.end(), [](unsigned char c) { return !std::isspace(c); });
    return std::string(it, str.end());
}

std::string StringUtils::TrimRight(const std::string& str)
{
    auto it = std::find_if(str.rbegin(), str.rend(), [](unsigned char c) { return !std::isspace(c); });
    return std::string(str.begin(), it.base());
}

std::string StringUtils::Trim(const std::string& str)
{
    return TrimLeft(TrimRight(str));
}

std::vector<std::string> StringUtils::Split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter))
        result.push_back(item);
    return result;
}

std::string StringUtils::Join(const std::vector<std::string>& parts, const std::string& delimiter)
{
    if (parts.empty())
        return "";

    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i)
        result += delimiter + parts[i];
    return result;
}

bool StringUtils::StartsWith(const std::string& str, const std::string& prefix)
{
    return str.length() >= prefix.length() && str.substr(0, prefix.length()) == prefix;
}

bool StringUtils::EndsWith(const std::string& str, const std::string& suffix)
{
    return str.length() >= suffix.length() && str.substr(str.length() - suffix.length()) == suffix;
}

bool StringUtils::ContainsIgnoreCase(const std::string& str, const std::string& substr)
{
    return ToLower(str).find(ToLower(substr)) != std::string::npos;
}

std::string StringUtils::ReplaceAll(const std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
        return str;

    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos)
    {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

std::string StringUtils::SubstituteTokens(std::string text, const std::map<std::string, std::string>& tokens)
{
    for (const auto& [key, value] : tokens)
        text = ReplaceAll(text, "{" + key + "}", value);
    return text;
}

bool StringUtils::IsNumeric(const std::string& str)
{
    if (str.empty())
        return false;
    return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isdigit(c); });
}

StringUtils::TargetInfo StringUtils::ParseTarget(const std::string& target)
{
    if (target.empty())
        return {TargetType::Name, ""};

    if (target == "@all" || target == "@*")
        return {TargetType::All, target};

    if (target == "@me")
        return {TargetType::Me, target};

    if (target[0] == '#' && target.length() > 1)
    {
        auto indexStr = target.substr(1);
        if (IsNumeric(indexStr))
            return {TargetType::Index, indexStr};
    }

    if (IsNumeric(target) && target.length() >= 15)
    {
        try
        {
            int64_t steamId64 = std::stoll(target);
            if (SteamId::IsValid(steamId64))
                return {TargetType::SteamId, target};
        }
        catch (...)
        {}
    }

    if (StartsWith(target, "[U:1:"))
    {
        auto steamId64 = SteamId::FromSteamId3(target);
        if (steamId64.has_value())
            return {TargetType::SteamId, std::to_string(*steamId64)};
    }

    if (StartsWith(target, "STEAM_"))
    {
        auto steamId64 = SteamId::FromSteamId(target);
        if (steamId64.has_value())
            return {TargetType::SteamId, std::to_string(*steamId64)};
    }

    return {TargetType::Name, target};
}

}  // namespace CS2Kit::Utils
