#pragma once

#include <CS2Kit/Players/Player.hpp>
#include <functional>
#include <string>
#include <vector>

namespace CS2Kit::Players
{

/** Targetability policy injected by the consumer (immunity, same-team rules, ...), mirroring
 *  CommandManager's permission callback. An empty function allows every caller->target pair. */
using CanTargetFn = std::function<bool(Player& caller, Player& target)>;

/** One matched player from @ref ResolveTargets. */
struct ResolvedTarget
{
    Player* Target = nullptr;
    bool Allowed = true; /**< false when the CanTargetFn policy blocked this match. */
};

/**
 * Resolve a target token to the list of matching online players.
 *
 * Supported syntax (parsed by CS2Kit::Utils::StringUtils::ParseTarget):
 *   - `@all` / `@*` - every connected player.
 *   - `@me` - the caller.
 *   - `#N` - slot index N (e.g. "#3").
 *   - a SteamID64 / `STEAM_...` / `[U:1:...]` - that exact player.
 *   - anything else - case-insensitive substring of the player's display name.
 *
 * Each match carries the policy verdict in `Allowed` so the caller can report "X is immune".
 * A null @p caller (server console) is always allowed. Returns empty when nothing matched.
 */
std::vector<ResolvedTarget> ResolveTargets(const std::string& token, Player* caller, const CanTargetFn& canTarget = {});

enum class SingleTargetError
{
    None,
    NoMatch,   /**< Nothing matched the token. */
    Immune,    /**< Matches existed, but the policy blocked all of them. */
    Ambiguous, /**< More than one allowed match; the token must be narrowed. */
};

/** Result of narrowing a token to exactly one allowed player. */
struct SingleTargetResult
{
    Player* Target = nullptr; /**< Non-null iff Error == None. */
    SingleTargetError Error = SingleTargetError::None;
    int MatchCount = 0; /**< Allowed matches found - lets an Ambiguous message show the count. */
};

/** Narrow @p token to a single allowed player, preferring allowed matches over blocked ones. */
SingleTargetResult ResolveSingleTarget(const std::string& token, Player* caller, const CanTargetFn& canTarget = {});

}  // namespace CS2Kit::Players
