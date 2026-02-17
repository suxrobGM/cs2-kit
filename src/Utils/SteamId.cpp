#include "SteamId.hpp"

#include <format>
#include <regex>

namespace CS2Kit::Utils
{

std::string SteamId::ToSteamId3(int64_t steamId64)
{
    uint32_t accountId = GetAccountId(steamId64);
    return std::format("[U:1:{}]", accountId);
}

std::string SteamId::ToSteamId(int64_t steamId64)
{
    uint32_t accountId = GetAccountId(steamId64);
    uint32_t authServer = accountId % 2;
    uint32_t accountNum = accountId / 2;
    return std::format("STEAM_0:{}:{}", authServer, accountNum);
}

std::optional<int64_t> SteamId::FromSteamId3(const std::string& steamId3)
{
    std::regex pattern(R"(\[U:1:(\d+)\])");
    std::smatch matches;
    if (std::regex_match(steamId3, matches, pattern))
    {
        try
        {
            uint32_t accountId = std::stoul(matches[1].str());
            return SteamId64Base + accountId;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<int64_t> SteamId::FromSteamId(const std::string& steamId)
{
    std::regex pattern(R"(STEAM_[0-5]:([01]):(\d+))");
    std::smatch matches;
    if (std::regex_match(steamId, matches, pattern))
    {
        try
        {
            uint32_t authServer = std::stoul(matches[1].str());
            uint32_t accountNum = std::stoul(matches[2].str());
            uint32_t accountId = accountNum * 2 + authServer;
            return SteamId64Base + accountId;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

bool SteamId::IsValid(int64_t steamId64)
{
    return steamId64 >= SteamId64Base && steamId64 < (SteamId64Base + 0x100000000LL);
}

uint32_t SteamId::GetAccountId(int64_t steamId64)
{
    return static_cast<uint32_t>(steamId64 - SteamId64Base);
}

}  // namespace CS2Kit::Utils
