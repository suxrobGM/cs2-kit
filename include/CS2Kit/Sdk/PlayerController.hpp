#pragma once

#include <cstdint>

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
