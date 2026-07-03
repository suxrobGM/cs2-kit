#include "Sdk/Schema.hpp"

#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Sdk/MemoryAccess.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <bit>
#include <entity2/concreteentitylist.h>
#include <entity2/entityidentity.h>
#include <entity2/entityinstance.h>
#include <entity2/entitysystem.h>

using CS2Kit::Core::Engine;

// The SDK's entity2 sources (entitykeyvalues.cpp) link against this accessor;
// route it to the kit's resolved entity system so both agree on the pointer.
CGameEntitySystem* GameEntitySystem()
{
    auto* services = CS2Kit::Core::EngineOrNull();
    return services ? services->Entities.GetEntitySystem() : nullptr;
}

namespace CS2Kit::Sdk
{
using namespace CS2Kit::Utils;

void EntitySystem::ResolveSchemaOffsets()
{
    if (_schemaOffsetsResolved)
        return;

    auto& schema = Engine().Schema();

    _offsetPlayerPawn = schema.GetOffset("CBasePlayerController", "m_hPawn");
    _offsetMovementServices = schema.GetOffset("CBasePlayerPawn", "m_pMovementServices");
    _offsetButtons = schema.GetOffset("CPlayer_MovementServices", "m_nButtons");
    _offsetButtonStates = schema.GetOffset("CInButtonState", "m_pButtonStates");

    _schemaOffsetsResolved = true;
}

CGameEntitySystem* EntitySystem::ReadEntitySystemPointer()
{
    auto& interfaces = Engine().Interfaces;
    if (!interfaces.GameResourceService)
        return nullptr;

    // "GameEntitySystem" = byte offset of the CGameEntitySystem* cached inside CGameResourceService.
    int offsetGameEntitySystem = Engine().GameData.GetOffset("GameEntitySystem");
    if (offsetGameEntitySystem < 0)
        return nullptr;

    return ReadAt<CGameEntitySystem*>(interfaces.GameResourceService, offsetGameEntitySystem);
}

bool EntitySystem::Initialize()
{
    auto& interfaces = Engine().Interfaces;

    if (!interfaces.GameResourceService)
    {
        Log::Warn("IGameResourceService not available.");
    }

    int offsetGameEntitySystem = Engine().GameData.GetOffset("GameEntitySystem");

    if (offsetGameEntitySystem < 0)
    {
        Log::Warn("GameEntitySystem offset not found in gamedata.");
    }
    else
    {
        Log::Info("Gamedata loaded (entity system offset: {}).", offsetGameEntitySystem);
    }

    interfaces.EntitySystem = ReadEntitySystemPointer();

    if (interfaces.EntitySystem)
    {
        Log::Info("Entity system initialized.");
    }

    return true;
}

CGameEntitySystem* EntitySystem::GetEntitySystem()
{
    auto& interfaces = Engine().Interfaces;

    if (!interfaces.EntitySystem)
        interfaces.EntitySystem = ReadEntitySystemPointer();

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
    if (handle == 0xFFFFFFFF)  // INVALID_EHANDLE_INDEX: unset/cleared handle
        return nullptr;

    int entryIndex = handle & 0x7FFF;  // low 15 bits = entity index, high bits = serial number

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

    // Controllers occupy entity indices 1..MaxPlayers (index 0 is worldspawn).
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

    uint32_t hPawn = ReadAt<uint32_t>(pController, _offsetPlayerPawn);
    CEntityInstance* pPawn = ResolveEntityHandle(hPawn);
    if (!pPawn)
        return 0;

    auto* pMovementServices = ReadAt<uint8_t*>(pPawn, _offsetMovementServices);
    if (!pMovementServices)
        return 0;

    auto* pButtonStates = MemberPtr<uint64_t>(pMovementServices, _offsetButtons + _offsetButtonStates);

    return pButtonStates[0];  // m_pButtonStates is uint64[3]: [0] held, [1] changed, [2] scroll
}

bool EntitySystem::IsPlayerSlotValid(int slot)
{
    return GetPlayerController(slot) != nullptr;
}

namespace
{
// Prototypes mirror CS2Fixes' src/addresses.h; re-verify there after CS2 updates.
using FindByClassNameFn = CEntityInstance* (*)(CEntitySystem* system, CEntityInstance* startAfter,
                                               const char* className);
using FindByNameFn = CEntityInstance* (*)(CEntitySystem* system, CEntityInstance* startAfter, const char* name,
                                          CEntityInstance* searching, CEntityInstance* activator,
                                          CEntityInstance* caller, void* filter);
}  // namespace

void EntitySystem::ResolveFinderSignatures()
{
    if (_findersResolved)
        return;

    auto& gameData = Engine().GameData;
    _findByClassName = gameData.FindSignature("CGameEntitySystem_FindEntityByClassName");
    _findByName = gameData.FindSignature("CGameEntitySystem_FindEntityByName");

    if (!_findByClassName || !_findByName)
        Log::Warn("Entity finder signature(s) not resolved; FindByClassName/FindByName are disabled.");

    _findersResolved = true;
}

CEntityInstance* EntitySystem::FindByClassName(CEntityInstance* startAfter, const char* className)
{
    ResolveFinderSignatures();

    auto* pSys = GetEntitySystem();
    if (!_findByClassName || !pSys || !className)
        return nullptr;

    return std::bit_cast<FindByClassNameFn>(_findByClassName)(pSys, startAfter, className);
}

CEntityInstance* EntitySystem::FindByName(CEntityInstance* startAfter, const char* name)
{
    ResolveFinderSignatures();

    auto* pSys = GetEntitySystem();
    if (!_findByName || !pSys || !name)
        return nullptr;

    return std::bit_cast<FindByNameFn>(_findByName)(pSys, startAfter, name, nullptr, nullptr, nullptr, nullptr);
}

}  // namespace CS2Kit::Sdk
