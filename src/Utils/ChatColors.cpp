#include <CS2Kit/Utils/ChatColors.hpp>

#include <cctype>
#include <unordered_map>

namespace CS2Kit::Utils::ChatColors
{

std::string_view ParseNamed(std::string_view name)
{
    if (name.empty())
        return Default;

    std::string lower;
    lower.reserve(name.size());
    for (char c : name)
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

    static const std::unordered_map<std::string, std::string_view> kTable = {
        {"default", Default},  {"white", Default},        {"red", Red},
        {"lightpurple", LightPurple},                     {"green", Green},
        {"olive", Olive},      {"lime", Lime},            {"lightred", LightRed},
        {"silver", LightRed},  {"gray", Gray},            {"grey", Gray},
        {"lightyellow", LightYellow},                     {"lightblue", LightBlue},
        {"blue", Blue},        {"purple", Purple},        {"pink", Pink},
        {"gold", Gold},        {"orange", Gold},          {"yellow", Yellow},
    };

    auto it = kTable.find(lower);
    return it != kTable.end() ? it->second : Default;
}

std::string Strip(std::string_view text)
{
    std::string out;
    out.reserve(text.size());
    for (char c : text)
    {
        if (static_cast<unsigned char>(c) > 0x10)
            out.push_back(c);
    }
    return out;
}

}  // namespace CS2Kit::Utils::ChatColors
