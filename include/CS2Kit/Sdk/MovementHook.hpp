#pragma once

#include <CS2Kit/Core/CallbackRegistry.hpp>
#include <cstdint>
#include <functional>

namespace CS2Kit::Sdk
{

/**
 * @brief Inline detour on `CCSPlayer_MovementServices::ProcessMovement` - the per-tick,
 * per-player movement entry point (gamedata signature "ProcessMovement").
 *
 * ProcessMovement is where the actual movement simulation runs (jump/stamina/friction/
 * velocity), so a pre/post pair brackets exactly the code that reads the movement convars.
 * That makes it the place for per-player state flips (see RawConVar) that must be visible
 * to the engine's own movement code - e.g. server-side auto-bunnyhop for one player.
 *
 * Install() resolves the signature and patches the function; it needs no live pawn and can
 * be called from OnLoad or lazily. The detour is removed on destruction (plugin unload).
 *
 * Pre/post callbacks fire around each player's ProcessMovement with the owning slot
 * resolved (-1 when unresolved).
 *
 * The byte pattern is gamedata-maintained and drifts with CS2 updates; a failed match
 * just disables the hook (Install() returns false), but re-verify the pattern against
 * SwiftlyS2/CS2Fixes gamedata after every game update. Only one plugin per server should
 * install this detour: stacked inline hooks from separately unloadable modules are only
 * safe to remove in LIFO order.
 */
class MovementHook
{
public:
    using Callback = std::function<void(int slot)>;

    MovementHook() = default;
    ~MovementHook() { Remove(); }
    MovementHook(const MovementHook&) = delete;
    MovementHook& operator=(const MovementHook&) = delete;

    /** Install the detour. False when the gamedata signature is missing or patching failed. */
    bool Install();
    bool Installed() const { return _installed; }
    void Remove();

    uint64_t ListenPre(Callback callback) { return _pre.Add(std::move(callback)); }
    uint64_t ListenPost(Callback callback) { return _post.Add(std::move(callback)); }
    void RemoveListener(uint64_t id);

    /** Slot whose pawn owns @p movementServices, or -1. */
    int SlotFromMovementServices(void* movementServices) const;

    /** @internal Detour body: pre listeners, original ProcessMovement, post listeners. */
    void OnProcessMovement(void* movementServices, void* moveData);

private:
    Core::CallbackRegistry<Callback> _pre;
    Core::CallbackRegistry<Callback> _post;
    bool _installed = false;
};

}  // namespace CS2Kit::Sdk
