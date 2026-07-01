#include "Sdk/Schema.hpp"
#include "Sdk/VirtualCall.hpp"

#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/EntityRender.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Sdk/PlayerController.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <cstring>
#include <eiface.h>
#include <entity2/entityinstance.h>
#include <mathlib/vector.h>

using CS2Kit::Core::Engine;

namespace CS2Kit::Sdk
{

using namespace CS2Kit::Utils;

namespace
{
// Resolve a vtable index by its gamedata name and call it on `target`. No-op (with a warning)
// when the offset is missing or `target` is null - collapses the lookup/guard/dispatch the
// vtable wrappers all repeat.
template <typename... Args>
void CallVtableByName(void* target, const char* name, Args... args)
{
    if (!target)
        return;
    int index = Engine().GameData.GetOffset(name);
    if (index < 0)
    {
        Log::Warn("PlayerController: vtable offset '{}' not found.", name);
        return;
    }
    CallVirtual<void>(index, target, args...);
}
}  // namespace

PlayerController::PlayerController(int slot) : _slot(slot)
{
    _controller = Engine().Entities.GetPlayerController(slot);
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

    int offset = Engine().Schema().GetOffset("CCSPlayerController", "m_hPlayerPawn");
    if (offset < 0)
        return nullptr;

    auto hPawn = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(_controller) + offset);
    return Engine().Entities.ResolveEntityHandle(hPawn);
}

void PlayerController::Kick(const char* reason) const
{
    if (!IsValid())
        return;

    auto* engine = Engine().Interfaces.Engine;
    if (!engine)
    {
        Log::Warn("PlayerController::Kick: IVEngineServer2 not available.");
        return;
    }

    engine->DisconnectClient(CPlayerSlot(_slot), NETWORK_DISCONNECT_KICKED, reason);
}

int PlayerController::SchemaOffset(const char* className, const char* fieldName) const
{
    return Engine().Schema().GetOffset(className, fieldName);
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
    return Engine().Entities.GetPlayerButtons(_slot);
}

void PlayerController::SetHealth(int health) const
{
    SetPawnField<int>("CBaseEntity", "m_iHealth", health);
}

int PlayerController::GetArmor() const
{
    return GetPawnField<int>("CCSPlayerPawn", "m_ArmorValue");
}

void PlayerController::SetArmor(int armor) const
{
    SetPawnField<int>("CCSPlayerPawn", "m_ArmorValue", armor);
}

uint32_t PlayerController::GetFlags() const
{
    return GetPawnField<uint32_t>("CBaseEntity", "m_fFlags");
}

void PlayerController::SetFlags(uint32_t flags) const
{
    SetPawnField<uint32_t>("CBaseEntity", "m_fFlags", flags);
}

Vector PlayerController::GetVelocity() const
{
    return GetPawnField<Vector>("CBaseEntity", "m_vecAbsVelocity");
}

void PlayerController::SetVelocity(const Vector& velocity) const
{
    SetPawnField<Vector>("CBaseEntity", "m_vecAbsVelocity", velocity);
}

uint8_t PlayerController::GetRenderMode() const
{
    return GetPawnField<uint8_t>("CBaseModelEntity", "m_nRenderMode");
}

uint32_t PlayerController::GetRenderColor() const
{
    return GetPawnField<uint32_t>("CBaseModelEntity", "m_clrRender");
}

void PlayerController::SetRender(uint8_t mode, uint32_t color) const
{
    SetPawnField<uint8_t>("CBaseModelEntity", "m_nRenderMode", mode);
    SetPawnField<uint32_t>("CBaseModelEntity", "m_clrRender", color);
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
    CallVtableByName(GetPawn(), "CommitSuicide", false, true);
}

void PlayerController::ChangeTeam(int team) const
{
    CallVtableByName(_controller, "ChangeTeam", team);
}

void PlayerController::Respawn() const
{
    CallVtableByName(_controller, "Respawn");
}

void PlayerController::Teleport(const Vector* origin, const QAngle* angles, const Vector* velocity) const
{
    CallVtableByName(GetPawn(), "Teleport", origin, angles, velocity);
}

namespace
{
// Player name is stored as a 128-byte fixed buffer on CBasePlayerController.
constexpr size_t PlayerNameBufferSize = 128;
}  // namespace

MoveType PlayerController::GetMoveType() const
{
    return static_cast<MoveType>(GetPawnField<uint8_t>("CBaseEntity", "m_MoveType"));
}

void PlayerController::SetMoveType(MoveType type) const
{
    auto value = static_cast<uint8_t>(type);
    SetPawnField<uint8_t>("CBaseEntity", "m_MoveType", value);
    SetPawnField<uint8_t>("CBaseEntity", "m_nActualMoveType", value);
}

ObserverMode_t PlayerController::GetObserverMode() const
{
    return static_cast<ObserverMode_t>(GetPawnField<uint8_t>("CPlayer_ObserverServices", "m_iObserverMode"));
}

void PlayerController::SetObserverMode(ObserverMode_t mode) const
{
    SetPawnField<uint8_t>("CPlayer_ObserverServices", "m_iObserverMode", static_cast<uint8_t>(mode));
}

std::string PlayerController::GetPlayerName() const
{
    if (!_controller)
        return {};

    int offset = Engine().Schema().GetOffset("CBasePlayerController", "m_iszPlayerName");
    if (offset < 0)
        return {};

    auto* p = reinterpret_cast<const char*>(reinterpret_cast<uint8_t*>(_controller) + offset);
    size_t len = 0;
    while (len < PlayerNameBufferSize && p[len] != '\0')
        ++len;
    return std::string(p, len);
}

void PlayerController::SetPlayerName(const std::string& name) const
{
    if (!_controller)
        return;

    int offset = Engine().Schema().GetOffset("CBasePlayerController", "m_iszPlayerName");
    if (offset < 0)
        return;

    auto* dst = reinterpret_cast<char*>(reinterpret_cast<uint8_t*>(_controller) + offset);
    std::memset(dst, 0, PlayerNameBufferSize);
    size_t copyLen = name.size();
    if (copyLen >= PlayerNameBufferSize)
        copyLen = PlayerNameBufferSize - 1;
    if (copyLen > 0)
        std::memcpy(dst, name.data(), copyLen);
}

void PlayerController::SetVisible(bool visible, uint8_t alpha) const
{
    auto* pawn = GetPawn();
    if (!pawn)
        return;

    RenderMode_t mode = visible ? RenderMode_t::Normal : RenderMode_t::TransTexture;
    uint32_t color = visible ? ColorOpaqueWhite : ((static_cast<uint32_t>(alpha) << 24) | 0x00FFFFFFu);

    SetEntityRender(pawn, mode, color);
}

}  // namespace CS2Kit::Sdk
