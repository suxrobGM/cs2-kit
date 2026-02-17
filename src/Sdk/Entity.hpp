#pragma once

#include "../Core/Singleton.hpp"

#include <cstdint>
#include <string>

class CGameEntitySystem;
class CEntityInstance;
class CEntityIdentity;

namespace CS2Kit::Sdk
{

/** @defgroup ButtonFlags Player Button Flags */
/** @{ */
constexpr uint64_t IN_ATTACK = 0x1;
constexpr uint64_t IN_JUMP = 0x2;
constexpr uint64_t IN_DUCK = 0x4;
constexpr uint64_t IN_FORWARD = 0x8;
constexpr uint64_t IN_BACK = 0x10;
constexpr uint64_t IN_USE = 0x20;
constexpr uint64_t IN_TURNLEFT = 0x80;
constexpr uint64_t IN_TURNRIGHT = 0x100;
constexpr uint64_t IN_MOVELEFT = 0x200;
constexpr uint64_t IN_MOVERIGHT = 0x400;
constexpr uint64_t IN_ATTACK2 = 0x800;
constexpr uint64_t IN_RELOAD = 0x2000;
constexpr uint64_t IN_SPEED = 0x10000;
constexpr uint64_t IN_SCORE = 0x200000000ULL;
constexpr uint64_t IN_ZOOM = 0x400000000ULL;
constexpr uint64_t IN_LOOK_AT_WEAPON = 0x800000000ULL;
/** @} */

constexpr int MaxPlayers = 64;

/**
 * Entity system access layer for the Source 2 engine.
 * Resolves CGameEntitySystem from IGameResourceService, provides player
 * controller lookup by slot, entity handle resolution, and button state reading.
 */
class EntitySystem : public Core::Singleton<EntitySystem>
{
public:
    explicit EntitySystem(Token) {}

    bool Initialize();
    CGameEntitySystem* GetEntitySystem();
    CEntityInstance* GetPlayerController(int slot);
    CEntityInstance* ResolveEntityHandle(uint32_t handle);
    uint64_t GetPlayerButtons(int slot);
    bool IsPlayerSlotValid(int slot);

private:
    void ResolveSchemaOffsets();
    CEntityIdentity* GetEntityIdentityByIndex(CGameEntitySystem* pSys, int index);

    int _offsetPlayerPawn = -1;
    int _offsetMovementServices = -1;
    int _offsetButtons = -1;
    int _offsetButtonStates = -1;
    bool _schemaOffsetsResolved = false;
};

}  // namespace CS2Kit::Sdk
