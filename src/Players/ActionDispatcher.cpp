#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Players/ActionDispatcher.hpp>
#include <CS2Kit/Players/PlayerManager.hpp>

namespace CS2Kit::Players
{

using Sdk::PlayerController;

ActionContext ActionDispatcher::Resolve(int callerSlot, int targetSlot, const std::string& permission) const
{
    ActionContext ctx{nullptr, nullptr, PlayerController(callerSlot), PlayerController(targetSlot)};

    auto& plrMgr = Core::Engine().Players;
    ctx.Caller = plrMgr.GetPlayerBySlot(callerSlot);
    ctx.Target = plrMgr.GetPlayerBySlot(targetSlot);

    if (!ctx.Caller || !ctx.Target)
        return ctx;

    if (!permission.empty() && _hasPermission && !_hasPermission(*ctx.Caller, permission))
    {
        ctx.Caller = nullptr;
        return ctx;
    }
    if (_canTarget && !_canTarget(*ctx.Caller, *ctx.Target))
    {
        ctx.Caller = nullptr;
        return ctx;
    }
    return ctx;
}

void ActionDispatcher::Run(int callerSlot, int targetSlot, const Action& action) const
{
    auto ctx = Resolve(callerSlot, targetSlot, action.Permission);
    if (!ctx.Valid())
        return;
    if (action.RequireAlive && !ctx.TargetCtrl.IsAlive())
        return;
    if (auto key = action.Body(ctx))
        Broadcast(ctx, *key);
}

void ActionDispatcher::Run(int callerSlot, int targetSlot, int param, const ParamAction& action) const
{
    auto ctx = Resolve(callerSlot, targetSlot, action.Permission);
    if (!ctx.Valid())
        return;
    if (action.RequireAlive && !ctx.TargetCtrl.IsAlive())
        return;
    if (auto key = action.Body(ctx, param))
        Broadcast(ctx, *key);
}

void ActionDispatcher::Broadcast(const ActionContext& ctx, const std::string& translationKey) const
{
    if (!_broadcast || !ctx.Caller || !ctx.Target)
        return;
    _broadcast(ctx, translationKey);
}

}  // namespace CS2Kit::Players
