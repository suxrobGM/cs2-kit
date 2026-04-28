#include "Sdk/Schema.hpp"
#include "Sdk/VirtualCall.hpp"

#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Sdk/PlayerController.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <eiface.h>
#include <entity2/entityinstance.h>
#include <mathlib/vector.h>

namespace CS2Kit::Sdk
{

using namespace CS2Kit::Utils;

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

template <typename T>
void PlayerController::SetField(const char* className, const char* fieldName, const T& value) const
{
    if (!_controller)
        return;

    int offset = SchemaService::Instance().GetOffset(className, fieldName);
    if (offset < 0)
        return;

    *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(_controller) + offset) = value;
}

template <typename T>
void PlayerController::SetPawnField(const char* className, const char* fieldName, const T& value) const
{
    auto* pawn = GetPawn();
    if (!pawn)
        return;

    int offset = SchemaService::Instance().GetOffset(className, fieldName);
    if (offset < 0)
        return;

    *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(pawn) + offset) = value;
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

Vector PlayerController::GetAbsOrigin() const
{
    return GetPawnField<Vector>("CBaseEntity", "m_vecAbsOrigin");
}

QAngle PlayerController::GetAbsAngles() const
{
    return GetPawnField<QAngle>("CBaseEntity", "m_angRotation");
}

QAngle PlayerController::GetEyeAngles() const
{
    return GetPawnField<QAngle>("CCSPlayerPawnBase", "m_angEyeAngles");
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

void PlayerController::Teleport(const Vector* origin, const QAngle* angles, const Vector* velocity) const
{
    auto* pawn = GetPawn();
    if (!pawn)
        return;

    int vtableIndex = GameData::Instance().GetOffset("Teleport");
    if (vtableIndex < 0)
    {
        Log::Warn("PlayerController::Teleport: Teleport vtable offset not found.");
        return;
    }

    CallVirtual<void>(vtableIndex, pawn, origin, angles, velocity);
}

// Explicit template instantiations for common types
template int PlayerController::GetField<int>(const char*, const char*) const;
template uint8_t PlayerController::GetField<uint8_t>(const char*, const char*) const;
template uint32_t PlayerController::GetField<uint32_t>(const char*, const char*) const;
template float PlayerController::GetField<float>(const char*, const char*) const;
template Vector PlayerController::GetField<Vector>(const char*, const char*) const;
template QAngle PlayerController::GetField<QAngle>(const char*, const char*) const;

template int PlayerController::GetPawnField<int>(const char*, const char*) const;
template uint8_t PlayerController::GetPawnField<uint8_t>(const char*, const char*) const;
template uint32_t PlayerController::GetPawnField<uint32_t>(const char*, const char*) const;
template float PlayerController::GetPawnField<float>(const char*, const char*) const;
template Vector PlayerController::GetPawnField<Vector>(const char*, const char*) const;
template QAngle PlayerController::GetPawnField<QAngle>(const char*, const char*) const;

template void PlayerController::SetField<int>(const char*, const char*, const int&) const;
template void PlayerController::SetField<uint8_t>(const char*, const char*, const uint8_t&) const;
template void PlayerController::SetField<uint32_t>(const char*, const char*, const uint32_t&) const;
template void PlayerController::SetField<float>(const char*, const char*, const float&) const;
template void PlayerController::SetField<Vector>(const char*, const char*, const Vector&) const;
template void PlayerController::SetField<QAngle>(const char*, const char*, const QAngle&) const;

template void PlayerController::SetPawnField<int>(const char*, const char*, const int&) const;
template void PlayerController::SetPawnField<uint8_t>(const char*, const char*, const uint8_t&) const;
template void PlayerController::SetPawnField<uint32_t>(const char*, const char*, const uint32_t&) const;
template void PlayerController::SetPawnField<float>(const char*, const char*, const float&) const;
template void PlayerController::SetPawnField<Vector>(const char*, const char*, const Vector&) const;
template void PlayerController::SetPawnField<QAngle>(const char*, const char*, const QAngle&) const;

}  // namespace CS2Kit::Sdk
