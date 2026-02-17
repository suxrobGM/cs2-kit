#include <CS2Kit/Sdk/Entity.hpp>

#include <CS2Kit/Utils/Log.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Sdk/Schema.hpp>

#include <entity2/concreteentitylist.h>
#include <entity2/entityidentity.h>
#include <entity2/entityinstance.h>
#include <entity2/entitysystem.h>

namespace CS2Kit::Sdk
{
using namespace CS2Kit::Utils;

void EntitySystem::ResolveSchemaOffsets()
{
    if (_schemaOffsetsResolved)
        return;

    auto& schema = SchemaService::Instance();
    _offsetPlayerPawn = schema.GetOffset("CCSPlayerController", "m_hPlayerPawn");
    _offsetMovementServices = schema.GetOffset("CBasePlayerPawn", "m_pMovementServices");
    _offsetButtons = schema.GetOffset("CPlayer_MovementServices", "m_nButtons");
    _offsetButtonStates = schema.GetOffset("CInButtonState", "m_pButtonStates");

    _schemaOffsetsResolved = true;

    if (_offsetPlayerPawn >= 0 && _offsetMovementServices >= 0 && _offsetButtons >= 0 && _offsetButtonStates >= 0)
    {
        Log::Info("Button access chain resolved via schema:");
        Log::Info("  Controller + 0x{:X} -> Pawn + 0x{:X} -> MovementServices + 0x{:X} -> Buttons + 0x{:X}",
                  _offsetPlayerPawn, _offsetMovementServices, _offsetButtons, _offsetButtonStates);
    }
    else
    {
        Log::Warn("Some schema offsets not resolved. Button detection may not work.");
    }
}

bool EntitySystem::Initialize()
{
    auto& interfaces = GameInterfaces::Instance();

    if (!interfaces.GameResourceService)
    {
        Log::Warn("IGameResourceService not available.");
    }

    int offsetGameEntitySystem = GameData::Instance().GetOffset("GameEntitySystem");

    if (offsetGameEntitySystem < 0)
    {
        Log::Warn("GameEntitySystem offset not found in gamedata.");
    }
    else
    {
        Log::Info("Gamedata loaded (entity system offset: {}).", offsetGameEntitySystem);
    }

    if (interfaces.GameResourceService && offsetGameEntitySystem >= 0)
    {
        interfaces.EntitySystem = *reinterpret_cast<CGameEntitySystem**>(
            reinterpret_cast<uintptr_t>(interfaces.GameResourceService) + offsetGameEntitySystem);
    }

    if (interfaces.EntitySystem)
    {
        Log::Info("Entity system initialized.");
    }
    else
    {
        Log::Warn("Entity system not available. Menu button detection disabled.");
    }

    return true;
}

CGameEntitySystem* EntitySystem::GetEntitySystem()
{
    auto& interfaces = GameInterfaces::Instance();

    if (!interfaces.EntitySystem && interfaces.GameResourceService)
    {
        int offsetGameEntitySystem = GameData::Instance().GetOffset("GameEntitySystem");
        if (offsetGameEntitySystem >= 0)
        {
            interfaces.EntitySystem = *reinterpret_cast<CGameEntitySystem**>(
                reinterpret_cast<uintptr_t>(interfaces.GameResourceService) + offsetGameEntitySystem);
        }
    }
    return interfaces.EntitySystem;
}

CEntityIdentity* EntitySystem::GetEntityIdentityByIndex(CGameEntitySystem* pSys, int index)
{
    if (!pSys || index < 0 || index >= MAX_TOTAL_ENTITIES)
        return nullptr;

    int chunk = index / MAX_ENTITIES_IN_LIST;
    int offset = index % MAX_ENTITIES_IN_LIST;

    CEntityIdentity* pChunk = pSys->m_EntityList.m_pIdentityChunks[chunk];
    if (!pChunk)
        return nullptr;

    return &pChunk[offset];
}

CEntityInstance* EntitySystem::ResolveEntityHandle(uint32_t handle)
{
    if (handle == 0xFFFFFFFF)
        return nullptr;

    int entryIndex = handle & 0x7FFF;

    auto* pSys = GetEntitySystem();
    if (!pSys)
        return nullptr;

    CEntityIdentity* pIdentity = GetEntityIdentityByIndex(pSys, entryIndex);
    if (!pIdentity)
        return nullptr;

    return pIdentity->m_pInstance;
}

CEntityInstance* EntitySystem::GetPlayerController(int slot)
{
    auto* pSys = GetEntitySystem();
    if (!pSys || slot < 0 || slot >= MaxPlayers)
        return nullptr;

    CEntityIdentity* pIdentity = GetEntityIdentityByIndex(pSys, slot + 1);
    if (!pIdentity)
        return nullptr;

    return pIdentity->m_pInstance;
}

uint64_t EntitySystem::GetPlayerButtons(int slot)
{
    if (!_schemaOffsetsResolved)
        ResolveSchemaOffsets();

    if (_offsetPlayerPawn < 0 || _offsetMovementServices < 0 || _offsetButtons < 0 || _offsetButtonStates < 0)
        return 0;

    CEntityInstance* pController = GetPlayerController(slot);
    if (!pController)
        return 0;

    auto* pCtrlBase = reinterpret_cast<uint8_t*>(pController);

    uint32_t hPawn = *reinterpret_cast<uint32_t*>(pCtrlBase + _offsetPlayerPawn);
    CEntityInstance* pPawn = ResolveEntityHandle(hPawn);
    if (!pPawn)
        return 0;

    auto* pPawnBase = reinterpret_cast<uint8_t*>(pPawn);

    auto* pMovementServices = *reinterpret_cast<uint8_t**>(pPawnBase + _offsetMovementServices);
    if (!pMovementServices)
        return 0;

    auto* pButtonStates = reinterpret_cast<uint64_t*>(pMovementServices + _offsetButtons + _offsetButtonStates);

    return pButtonStates[0];
}

bool EntitySystem::IsPlayerSlotValid(int slot)
{
    return GetPlayerController(slot) != nullptr;
}

}  // namespace CS2Kit::Sdk
