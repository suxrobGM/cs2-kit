#pragma once

#include <CS2Kit/Core/CallbackRegistry.hpp>
#include <cstdint>
#include <functional>

namespace CS2Kit::Sdk
{

/**
 * @brief Inline detours bracketing one player's movement processing (gamedata signatures
 * "ProcessUsercmds" and "ProcessMovement").
 *
 * Two detours cover the span where the engine consumes movement state:
 * `CCSPlayerController::ProcessUsercmds` (the per-player usercmd batch - subtick button
 * decomposition and jump handling live here) and `CCSPlayer_MovementServices::ProcessMovement`
 * (the movement simulation itself). ProcessMovement alone is NOT enough for convar-sensitive
 * behavior: the subtick jump code reads e.g. sv_autobunnyhopping outside it, which is why both
 * are bracketed.
 *
 * Listeners see one logical scope: when the calls nest, pre fires once on the outermost entry
 * and post once on the outermost exit, so a pre/post pair always brackets everything the engine
 * ran for that player. That makes the pair the place for per-player state flips (see RawConVar)
 * that the engine's own movement code must observe - e.g. server-side auto-bunnyhop for one
 * player.
 *
 * Install() resolves the signatures and patches; it needs no live pawn and can be called from
 * OnLoad or lazily. Detours are removed on destruction (plugin unload). Each signature fails
 * soft: a missed pattern logs a warning and skips that detour.
 *
 * The byte patterns are gamedata-maintained and drift with CS2 updates - re-verify against
 * SwiftlyS2/CS2Fixes gamedata after every game update. Only one plugin per server should
 * install these detours: stacked inline hooks from separately unloadable modules are only
 * safe to remove in LIFO order.
 */
class MovementHook
{
public:
    using Callback = std::function<void(int slot)>;

    /** Lifetime call counters, readable via a plugin debug command for live diagnosis. */
    struct Stats
    {
        uint64_t UsercmdsCalls = 0;   // ProcessUsercmds detour invocations
        uint64_t MovementCalls = 0;   // ProcessMovement detour invocations
        uint64_t ScopesEntered = 0;   // outermost scopes with a resolved slot
        uint64_t UnresolvedSlots = 0; // outermost scopes where slot resolution failed (-1)
    };

    MovementHook() = default;
    ~MovementHook() { Remove(); }
    MovementHook(const MovementHook&) = delete;
    MovementHook& operator=(const MovementHook&) = delete;

    /** Install the detours. False when nothing could be patched (all signatures missing). */
    bool Install();
    bool Installed() const { return _installed; }
    void Remove();

    uint64_t ListenPre(Callback callback) { return _pre.Add(std::move(callback)); }
    uint64_t ListenPost(Callback callback) { return _post.Add(std::move(callback)); }
    void RemoveListener(uint64_t id);

    const Stats& GetStats() const { return _stats; }

    /** Slot whose pawn owns @p movementServices, or -1. */
    int SlotFromMovementServices(void* movementServices) const;
    /** Slot whose controller is @p controller, or -1. */
    int SlotFromController(void* controller) const;

    /** @internal Detour bodies. */
    void OnProcessMovement(void* movementServices, void* moveData);
    void* OnProcessUsercmds(void* controller, void* cmds, int numcmds, bool paused, float margin);

private:
    void EnterScope(int slot);
    void ExitScope();

    Core::CallbackRegistry<Callback> _pre;
    Core::CallbackRegistry<Callback> _post;
    bool _installed = false;
    int _depth = 0;      // nesting depth: ProcessMovement usually runs inside ProcessUsercmds
    int _scopeSlot = -1; // slot resolved at the outermost entry, reused while nested
    Stats _stats;
};

}  // namespace CS2Kit::Sdk
