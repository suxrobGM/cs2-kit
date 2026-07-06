#pragma once

#include <CS2Kit/Core/CallbackRegistry.hpp>
#include <cstdint>
#include <functional>

namespace CS2Kit::Sdk
{

/**
 * @brief Brackets one player's movement processing with three hooks: inline detours on
 * `CCSPlayerController::ProcessUsercmds` (usercmd intake) and
 * `CCSPlayer_MovementServices::ProcessMovement` (gamedata signatures of the same names), plus
 * a vtable hook on `CPlayer_MovementServices::RunCommand` (gamedata offset "RunCommand") - the
 * deferred physics-phase entry that actually runs the movement simulation, jump checks
 * included. Live counter data showed the physics phase is NOT nested inside ProcessUsercmds,
 * so all three are needed to cover every place the engine reads movement convars.
 *
 * Listeners see one logical scope: when the calls nest, pre fires once on the outermost entry
 * and post once on the outermost exit, so a pre/post pair always brackets everything the engine
 * ran for that player. That makes the pair the place for per-player state flips (see RawConVar)
 * that the engine's own movement code must observe - e.g. server-side auto-bunnyhop for one
 * player.
 *
 * Install() resolves the signatures and patches immediately; the RunCommand vtable hook
 * additionally needs a live movement-services instance (a spawned pawn), so Install() may be
 * called repeatedly - it retries just that part until it succeeds (a PlayerSpawn listener is
 * the natural place). Each hook fails soft with a logged warning.
 *
 * The byte patterns and the vtable index are gamedata-maintained and drift with CS2 updates -
 * re-verify against SwiftlyS2/CS2Fixes gamedata after every game update (a wrong vtable index
 * calls an unrelated vfunc and crashes). Only one plugin per server should install the inline
 * detours: stacked inline hooks from separately unloadable modules are only safe to remove in
 * LIFO order.
 */
class MovementHook
{
public:
    using Callback = std::function<void(int slot)>;

    /** Lifetime call counters, readable via a plugin debug command for live diagnosis. */
    struct Stats
    {
        uint64_t UsercmdsCalls = 0;    // ProcessUsercmds detour invocations
        uint64_t MovementCalls = 0;    // ProcessMovement detour invocations
        uint64_t RunCommandCalls = 0;  // RunCommand vtable-hook invocations
        uint64_t ScopesEntered = 0;    // outermost scopes with a resolved slot
        uint64_t UnresolvedSlots = 0;  // outermost scopes where slot resolution failed (-1)
    };

    MovementHook() = default;
    ~MovementHook() { Remove(); }
    MovementHook(const MovementHook&) = delete;
    MovementHook& operator=(const MovementHook&) = delete;

    /** Install the hooks. Safe to call repeatedly: the RunCommand vtable part retries until a
     *  live pawn exists. False when nothing at all could be patched. */
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
    bool InstallRunCommandHook();
    void RemoveRunCommandHook();
    void* Hook_RunCommandPre(void* userCmd);
    void* Hook_RunCommandPost(void* userCmd);

    Core::CallbackRegistry<Callback> _pre;
    Core::CallbackRegistry<Callback> _post;
    bool _installed = false;
    bool _runCommandHooked = false;
    void* _vtable = nullptr;  // vtable of the RunCommand-hooked instance; see RemoveRunCommandHook()
    int _depth = 0;           // nesting depth across the three bracketing hooks
    int _scopeSlot = -1;      // slot resolved at the outermost entry, reused while nested
    Stats _stats;
};

}  // namespace CS2Kit::Sdk
