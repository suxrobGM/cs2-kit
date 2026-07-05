#pragma once

#include <CS2Kit/Core/CallbackRegistry.hpp>
#include <cstdint>
#include <functional>

namespace CS2Kit::Sdk
{

/**
 * @brief Manual vtable hook on CPlayer_MovementServices::RunCommand - the per-tick,
 * per-player movement entry point (gamedata offset "RunCommand").
 *
 * Install() needs at least one live movement-services instance (i.e. a spawned pawn), so
 * call it lazily - e.g. from a PlayerSpawn listener - and treat false as "retry later".
 * SourceHook patches the shared vtable, so hooking one instance covers every player.
 *
 * Pre/post callbacks fire around each player's RunCommand with the owning slot resolved
 * (-1 when unresolved). A pre/post pair brackets exactly that player's movement
 * processing, which makes it the place for per-player state flips (see RawConVar).
 *
 * Pre callbacks additionally receive the CUserCmd* about to be processed. The layout is
 * engine-defined: the CSGOUserCmdPB protobuf lives at +0x10 (verified against CS2Fixes and
 * SwiftlyS2). Mutating it before the command runs changes what the movement code consumes.
 *
 * The vtable index is gamedata-maintained and drifts with CS2 updates; a wrong index
 * calls an unrelated vfunc and crashes, so re-verify it after every update.
 */
class MovementHook
{
public:
    using PreCallback = std::function<void(int slot, void* userCmd)>;
    using Callback = std::function<void(int slot)>;

    MovementHook() = default;
    ~MovementHook() { Remove(); }
    MovementHook(const MovementHook&) = delete;
    MovementHook& operator=(const MovementHook&) = delete;

    /** Install the vtable hook. False when the gamedata offset is missing or no pawn is live yet. */
    bool Install();
    bool Installed() const { return _installed; }
    void Remove();

    uint64_t ListenPre(PreCallback callback) { return _pre.Add(std::move(callback)); }
    uint64_t ListenPost(Callback callback) { return _post.Add(std::move(callback)); }
    void RemoveListener(uint64_t id);

    /** Slot whose pawn owns @p movementServices, or -1. */
    int SlotFromMovementServices(void* movementServices) const;

private:
    void* Hook_RunCommandPre(void* userCmd);
    void* Hook_RunCommandPost(void* userCmd);

    Core::CallbackRegistry<PreCallback> _pre;
    Core::CallbackRegistry<Callback> _post;
    bool _installed = false;
    void* _vtable = nullptr;  // vtable of the hooked instance; see Remove()
    int _preSlot = -1;        // slot resolved in the pre hook, reused by the immediately-following post
};

}  // namespace CS2Kit::Sdk
