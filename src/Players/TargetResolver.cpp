#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Players/PlayerManager.hpp>
#include <CS2Kit/Players/TargetResolver.hpp>
#include <CS2Kit/Utils/StringUtils.hpp>
#include <charconv>
#include <cstdint>
#include <optional>

namespace CS2Kit::Players
{

using Utils::StringUtils;

namespace
{

/** from_chars over the whole string; nullopt on any parse failure (no exceptions, unlike stoll). */
template <typename T>
std::optional<T> ParseNumber(const std::string& text)
{
    T value{};
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;
    return value;
}

}  // namespace

std::vector<ResolvedTarget> ResolveTargets(const std::string& token, Player* caller, const CanTargetFn& canTarget)
{
    std::vector<ResolvedTarget> out;
    if (token.empty())
        return out;

    auto& mgr = Core::Engine().Players;

    auto add = [&](Player* p) {
        if (!p)
            return;
        ResolvedTarget r;
        r.Target = p;
        r.Allowed = (caller && canTarget) ? canTarget(*caller, *p) : true;
        out.push_back(r);
    };

    const auto info = StringUtils::ParseTarget(token);
    switch (info.Type)
    {
    case StringUtils::TargetType::All:
        for (auto* p : mgr.GetAllPlayers())
            add(p);
        break;
    case StringUtils::TargetType::Me:
        add(caller);
        break;
    case StringUtils::TargetType::Index:
        if (auto slot = ParseNumber<int>(info.Value))
            add(mgr.GetPlayerBySlot(*slot));
        break;
    case StringUtils::TargetType::SteamId:
        if (auto steamId = ParseNumber<int64_t>(info.Value))
            add(mgr.GetPlayerBySteamId(*steamId));
        break;
    case StringUtils::TargetType::Name:
    {
        const auto needle = StringUtils::ToLower(info.Value);
        for (auto* p : mgr.GetAllPlayers())
            if (p && StringUtils::ToLower(p->GetName()).find(needle) != std::string::npos)
                add(p);
        break;
    }
    }
    return out;
}

SingleTargetResult ResolveSingleTarget(const std::string& token, Player* caller, const CanTargetFn& canTarget)
{
    auto matches = ResolveTargets(token, caller, canTarget);
    if (matches.empty())
        return {.Error = SingleTargetError::NoMatch};

    // Prefer allowed targets; if every match is blocked by the policy, fail with that reason.
    std::vector<Player*> allowed;
    for (const auto& m : matches)
    {
        if (m.Allowed && m.Target)
            allowed.push_back(m.Target);
    }

    if (allowed.empty())
        return {.Error = SingleTargetError::Immune};
    if (allowed.size() > 1)
        return {.Error = SingleTargetError::Ambiguous, .MatchCount = static_cast<int>(allowed.size())};
    return {.Target = allowed[0], .MatchCount = 1};
}

}  // namespace CS2Kit::Players
