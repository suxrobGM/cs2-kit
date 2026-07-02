#pragma once

#include <CS2Kit/Players/Player.hpp>
#include <CS2Kit/Players/TargetResolver.hpp>
#include <CS2Kit/Sdk/PlayerController.hpp>
#include <functional>
#include <optional>
#include <string>

namespace CS2Kit::Players
{

/** A translation key returned by an action body, or nullopt to skip the broadcast. */
using OptKey = std::optional<std::string>;

/** Resolved caller/target pair handed to action bodies. */
struct ActionContext
{
    Player* Caller;
    Player* Target;
    Sdk::PlayerController CallerCtrl;
    Sdk::PlayerController TargetCtrl;

    bool Valid() const { return Caller && Target && TargetCtrl.IsValid(); }
};

/**
 * @brief A single-target player action expressed as data.
 *
 * Bodies receive a valid, permission/policy-checked @ref ActionContext, mutate the target, and
 * return the broadcast key (or nullopt to stay silent). @ref ActionDispatcher owns the
 * `Resolve -> guard -> Broadcast` shape, so an action is just its permission string, its guards,
 * and its effect - no per-action wrapper function.
 */
struct Action
{
    std::string Permission;    /**< Consumer-defined permission token; "" skips the check. */
    bool RequireAlive = false; /**< Skip silently if the target is dead. */
    std::function<OptKey(const ActionContext&)> Body;
};

/** Like @ref Action but carries an integer the menu/command supplies (health, team, ...). */
struct ParamAction
{
    std::string Permission;
    bool RequireAlive = false;
    std::function<OptKey(const ActionContext&, int param)> Body;
};

/**
 * @brief Runs data-defined actions with consumer-injected policy: a permission check, a
 * targetability check (immunity), and a broadcast sink. All three callbacks may be empty,
 * which skips the corresponding step.
 */
class ActionDispatcher
{
public:
    using PermissionFn = std::function<bool(Player& caller, const std::string& permission)>;
    using BroadcastFn = std::function<void(const ActionContext&, const std::string& translationKey)>;

    ActionDispatcher() = default;
    ActionDispatcher(PermissionFn hasPermission, CanTargetFn canTarget, BroadcastFn broadcast)
        : _hasPermission(std::move(hasPermission)), _canTarget(std::move(canTarget)), _broadcast(std::move(broadcast))
    {}

    /**
     * Resolve a caller+target slot pair, applying the permission and targetability policies.
     * Returns a context with `Valid() == false` if either player is missing, the caller lacks
     * @p permission (unless it is empty), or the targetability policy blocks the pair.
     */
    ActionContext Resolve(int callerSlot, int targetSlot, const std::string& permission) const;

    void Run(int callerSlot, int targetSlot, const Action& action) const;
    void Run(int callerSlot, int targetSlot, int param, const ParamAction& action) const;

    /** Invoke the broadcast sink directly (for bespoke flows like multi-target actions). */
    void Broadcast(const ActionContext& ctx, const std::string& translationKey) const;

private:
    PermissionFn _hasPermission;
    CanTargetFn _canTarget;
    BroadcastFn _broadcast;
};

}  // namespace CS2Kit::Players
