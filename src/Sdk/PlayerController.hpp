#pragma once

#include <cstdint>

class CEntityInstance;

namespace CS2Kit::Sdk
{

/**
 * Transient engine-side player wrapper around CCSPlayerController.
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

    int GetHealth() const;
    int GetTeam() const;
    int GetLifeState() const;
    bool IsAlive() const;
    uint64_t GetButtons() const;
    int GetArmor() const;

    void Slay() const;
    void ChangeTeam(int team) const;
    void Respawn() const;

private:
    int _slot;
    CEntityInstance* _controller = nullptr;
};

}  // namespace CS2Kit::Sdk
