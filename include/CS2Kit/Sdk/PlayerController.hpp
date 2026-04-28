#pragma once

#include <cstdint>
#include <string>

class CEntityInstance;
class Vector;
class QAngle;

namespace CS2Kit::Sdk
{

/**
 * @brief Transient engine-side player wrapper around CCSPlayerController.
 * Provides typed access to entity fields via the schema system,
 * engine operations (kick), and vtable operations (slay, change team, respawn).
 */
class PlayerController
{
public:
    explicit PlayerController(int slot);

    bool IsValid() const;
    CEntityInstance* GetEntity() const;
    CEntityInstance* GetPawn() const;
    int GetSlot() const { return _slot; }

    void Kick(const char* reason) const;

    template <typename T>
    T GetField(const char* className, const char* fieldName) const;

    template <typename T>
    T GetPawnField(const char* className, const char* fieldName) const;

    template <typename T>
    void SetField(const char* className, const char* fieldName, const T& value) const;

    template <typename T>
    void SetPawnField(const char* className, const char* fieldName, const T& value) const;

    int GetHealth() const;
    int GetTeam() const;
    int GetLifeState() const;
    bool IsAlive() const;
    uint64_t GetButtons() const;
    int GetArmor() const;

    Vector GetAbsOrigin() const;
    QAngle GetAbsAngles() const;
    QAngle GetEyeAngles() const;

    void Slay() const;
    void ChangeTeam(int team) const;
    void Respawn() const;

    /** Read `m_iObserverMode` from the pawn's CPlayer_ObserverServices. */
    int GetObserverMode() const;

    /** Write `m_iObserverMode` on the pawn's CPlayer_ObserverServices. */
    void SetObserverMode(uint8_t mode) const;

    /** Read `m_iszPlayerName` from the controller. Empty string if unavailable. */
    std::string GetPlayerName() const;

    /**
     * @brief Write `m_iszPlayerName` on the controller (128-byte fixed buffer).
     * Truncates to 127 chars + NUL. Replication to clients piggybacks on the
     * next state-change broadcast; pair with `ChangeTeam` or similar if you
     * need an immediate scoreboard refresh.
     */
    void SetPlayerName(const std::string& name) const;

    /**
     * @brief Apply transparency to the player pawn body. Weapons and wearables
     * are not affected (CS2 routes them through systems the server plugin
     * cannot reach). Pass `visible == true` to restore the default opaque state.
     *
     * @param visible  True to restore (mode=Normal, color=opaque white). False to hide.
     * @param alpha    Alpha byte applied when `visible == false`. Default 0 (fully invisible).
     */
    void SetVisible(bool visible, uint8_t alpha = 0) const;

    /**
     * @brief Teleport the pawn to an absolute origin/angles, optionally setting velocity.
     * Pass nullptr for any component to leave it unchanged. Calls CBaseEntity::Teleport
     * via the vtable offset registered as "Teleport" in gamedata.
     */
    void Teleport(const Vector* origin, const QAngle* angles, const Vector* velocity) const;

private:
    int _slot;
    CEntityInstance* _controller = nullptr;
};

}  // namespace CS2Kit::Sdk
