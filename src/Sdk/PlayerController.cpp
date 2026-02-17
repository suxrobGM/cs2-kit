#include "PlayerController.hpp"

#include "Entity.hpp"
#include "GameData.hpp"
#include "GameInterfaces.hpp"
#include "Schema.hpp"
#include "VirtualCall.hpp"
#include "../Utils/Log.hpp"

#include <eiface.h>
#include <entity2/entityinstance.h>

using namespace CS2Kit::Utils;

namespace CS2Kit::Sdk
{

PlayerController::PlayerController(int slot) : _slot(slot)
{
    _controller = EntitySystem::Instance().GetPlayerController(slot);
}

bool PlayerController::IsValid() const
{
    return _controller != nullptr;
}

CEntityInstance* PlayerController::GetEntity() const
{
    return _controller;
}

CEntityInstance* PlayerController::GetPawn() const
{
    if (!_controller)
        return nullptr;

    int offset = SchemaService::Instance().GetOffset("CCSPlayerController", "m_hPlayerPawn");
    if (offset < 0)
        return nullptr;

    auto hPawn = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(_controller) + offset);
    return EntitySystem::Instance().ResolveEntityHandle(hPawn);
}

void PlayerController::Kick(const char* reason) const
{
    if (!IsValid())
        return;

    auto* engine = GameInterfaces::Instance().Engine;
    if (!engine)
    {
        Log::Warn("PlayerController::Kick: IVEngineServer2 not available.");
        return;
    }

    engine->DisconnectClient(CPlayerSlot(_slot), NETWORK_DISCONNECT_KICKED, reason);
}

template <typename T>
T PlayerController::GetField(const char* className, const char* fieldName) const
{
    if (!_controller)
        return T{};

    int offset = SchemaService::Instance().GetOffset(className, fieldName);
    if (offset < 0)
        return T{};

    return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(_controller) + offset);
}

template <typename T>
T PlayerController::GetPawnField(const char* className, const char* fieldName) const
{
    auto* pawn = GetPawn();
    if (!pawn)
        return T{};

    int offset = SchemaService::Instance().GetOffset(className, fieldName);
    if (offset < 0)
        return T{};

    return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(pawn) + offset);
}

int PlayerController::GetHealth() const
{
    return GetPawnField<int>("CBaseEntity", "m_iHealth");
}

int PlayerController::GetTeam() const
{
    return GetPawnField<int>("CBaseEntity", "m_iTeamNum");
}

int PlayerController::GetLifeState() const
{
    return GetPawnField<uint8_t>("CBaseEntity", "m_lifeState");
}

bool PlayerController::IsAlive() const
{
    return GetPawn() != nullptr && GetLifeState() == 0;
}

uint64_t PlayerController::GetButtons() const
{
    return EntitySystem::Instance().GetPlayerButtons(_slot);
}

int PlayerController::GetArmor() const
{
    return GetPawnField<int>("CCSPlayerPawn", "m_ArmorValue");
}

void PlayerController::Slay() const
{
    auto* pawn = GetPawn();
    if (!pawn)
        return;

    int vtableIndex = GameData::Instance().GetOffset("CommitSuicide");
    if (vtableIndex < 0)
    {
        Log::Warn("PlayerController::Slay: CommitSuicide vtable offset not found.");
        return;
    }

    CallVirtual<void>(vtableIndex, pawn, false, true);
}

void PlayerController::ChangeTeam(int team) const
{
    if (!_controller)
        return;

    int vtableIndex = GameData::Instance().GetOffset("ChangeTeam");
    if (vtableIndex < 0)
    {
        Log::Warn("PlayerController::ChangeTeam: ChangeTeam vtable offset not found.");
        return;
    }

    CallVirtual<void>(vtableIndex, _controller, team);
}

void PlayerController::Respawn() const
{
    if (!_controller)
        return;

    int vtableIndex = GameData::Instance().GetOffset("Respawn");
    if (vtableIndex < 0)
    {
        Log::Warn("PlayerController::Respawn: Respawn vtable offset not found.");
        return;
    }

    CallVirtual<void>(vtableIndex, _controller);
}

// Explicit template instantiations for common types
template int PlayerController::GetField<int>(const char*, const char*) const;
template uint8_t PlayerController::GetField<uint8_t>(const char*, const char*) const;
template uint32_t PlayerController::GetField<uint32_t>(const char*, const char*) const;
template float PlayerController::GetField<float>(const char*, const char*) const;

template int PlayerController::GetPawnField<int>(const char*, const char*) const;
template uint8_t PlayerController::GetPawnField<uint8_t>(const char*, const char*) const;
template uint32_t PlayerController::GetPawnField<uint32_t>(const char*, const char*) const;
template float PlayerController::GetPawnField<float>(const char*, const char*) const;

}  // namespace CS2Kit::Sdk
