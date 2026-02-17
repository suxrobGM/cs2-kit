#pragma once

#include <CS2Kit/Core/Singleton.hpp>
#include <eiface.h>
#include <icvar.h>
#include <interfaces/interfaces.h>

// Forward declarations for HL2SDK types not exposed by standard includes
class CGameEntitySystem;
class IGameEventSystem;
class INetworkMessages;
class IGameEventManager2;
class ISchemaSystem;

namespace CS2Kit::Sdk
{

/**
 * @brief Centralized holder for all HL2SDK interface pointers.
 *
 * All fields are populated during Plugin::Load() via Metamod's
 * `GET_V_IFACE_ANY` / `GET_V_IFACE_CURRENT` macros.
 */
struct GameInterfaces : Core::Singleton<GameInterfaces>
{
    explicit GameInterfaces(Token) {}

    IServerGameDLL* ServerGameDLL = nullptr;
    IServerGameClients* ServerGameClients = nullptr;
    IVEngineServer2* Engine = nullptr;
    IGameEventSystem* GameEventSystem = nullptr;
    INetworkMessages* NetworkMessages = nullptr;
    IGameEventManager2* GameEventManager = nullptr;
    ISchemaSystem* SchemaSystem = nullptr;
    CGameEntitySystem* EntitySystem = nullptr;
    ICvar* CVar = nullptr;
    IGameResourceService* GameResourceService = nullptr;
};

}  // namespace CS2Kit::Sdk
