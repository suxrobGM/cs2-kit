#include "Core/ConsoleLogger.hpp"
#include "Core/Scheduler.hpp"
#include "Sdk/Schema.hpp"

#include <CS2Kit/CS2Kit.hpp>
#include <CS2Kit/Core/ILogger.hpp>
#include <CS2Kit/Core/Paths.hpp>
#include <CS2Kit/Menu/MenuManager.hpp>
#include <CS2Kit/Sdk/ConVarService.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/GameEventService.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <ISmmAPI.h>
#include <eiface.h>
#include <engine/igameeventsystem.h>
#include <icvar.h>
#include <interfaces/interfaces.h>
#include <networksystem/inetworkmessages.h>
#include <schemasystem/schemasystem.h>

namespace CS2Kit
{

static constexpr const char* DefaultGameDataPath = "addons/cs2-kit/gamedata/signatures.jsonc";
static Core::ConsoleLogger g_consoleLogger;

bool Initialize(ISmmAPI* ismm, char* error, size_t maxlen, const InitParams& params)
{
    // 1. Set up logging
    if (params.Logger)
    {
        Core::SetGlobalLogger(params.Logger);
    }
    else
    {
        g_consoleLogger.SetPrefix(params.LogPrefix);
        Core::SetGlobalLogger(&g_consoleLogger);
    }

    // 2. Set base directory for path resolution
    Core::SetBaseDir(ismm->GetBaseDir());

    Utils::Log::Info("Initializing CS2Kit...");

    // 3. Resolve SDK interfaces via Metamod
    auto resolveEngine = [&](const char* version) -> void* {
        return ismm->VInterfaceMatch(ismm->GetEngineFactory(), version, 0);
    };
    auto resolveServer = [&](const char* version) -> void* {
        return ismm->VInterfaceMatch(ismm->GetServerFactory(), version, 0);
    };

    auto& gi = Sdk::GameInterfaces::Instance();

    gi.ServerGameDLL = static_cast<IServerGameDLL*>(resolveServer(INTERFACEVERSION_SERVERGAMEDLL));
    if (!gi.ServerGameDLL)
    {
        ismm->Format(error, maxlen, "Could not find interface: %s", INTERFACEVERSION_SERVERGAMEDLL);
        return false;
    }

    gi.ServerGameClients = static_cast<IServerGameClients*>(resolveServer(INTERFACEVERSION_SERVERGAMECLIENTS));
    if (!gi.ServerGameClients)
    {
        ismm->Format(error, maxlen, "Could not find interface: %s", INTERFACEVERSION_SERVERGAMECLIENTS);
        return false;
    }

    gi.Engine = static_cast<IVEngineServer2*>(resolveEngine(INTERFACEVERSION_VENGINESERVER));
    if (!gi.Engine)
    {
        ismm->Format(error, maxlen, "Could not find interface: %s", INTERFACEVERSION_VENGINESERVER);
        return false;
    }

    gi.GameEventSystem = static_cast<IGameEventSystem*>(resolveEngine(GAMEEVENTSYSTEM_INTERFACE_VERSION));
    if (!gi.GameEventSystem)
    {
        ismm->Format(error, maxlen, "Could not find interface: %s", GAMEEVENTSYSTEM_INTERFACE_VERSION);
        return false;
    }

    gi.NetworkMessages = static_cast<INetworkMessages*>(resolveEngine(NETWORKMESSAGES_INTERFACE_VERSION));
    if (!gi.NetworkMessages)
    {
        ismm->Format(error, maxlen, "Could not find interface: %s", NETWORKMESSAGES_INTERFACE_VERSION);
        return false;
    }

    gi.SchemaSystem = static_cast<ISchemaSystem*>(resolveEngine(SCHEMASYSTEM_INTERFACE_VERSION));
    if (!gi.SchemaSystem)
    {
        ismm->Format(error, maxlen, "Could not find interface: %s", SCHEMASYSTEM_INTERFACE_VERSION);
        return false;
    }

    gi.CVar = static_cast<ICvar*>(resolveEngine(CVAR_INTERFACE_VERSION));
    if (!gi.CVar)
    {
        ismm->Format(error, maxlen, "Could not find interface: %s", CVAR_INTERFACE_VERSION);
        return false;
    }

    gi.GameResourceService =
        static_cast<IGameResourceService*>(resolveEngine(GAMERESOURCESERVICESERVER_INTERFACE_VERSION));
    if (!gi.GameResourceService)
    {
        ismm->Format(error, maxlen, "Could not find interface: %s", GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
        return false;
    }

    // 4. Set g_pCVar (required by HL2SDK's tier1/convar.cpp)
    g_pCVar = gi.CVar;

    // 5. Load game data (signatures and offsets)
    const char* gameDataPath = params.GameDataPath ? params.GameDataPath : DefaultGameDataPath;
    Utils::Log::Info("Loading game data from {}...", gameDataPath);
    Sdk::GameData::Instance().Load(gameDataPath);

    // 6. Initialize SDK subsystems
    Utils::Log::Info("Initializing SDK message system...");
    if (!Sdk::MessageSystem::Instance().Initialize())
    {
        Utils::Log::Error("Failed to initialize message system.");
        return false;
    }

    Utils::Log::Info("Initializing schema system...");
    if (!Sdk::SchemaService::Instance().Initialize())
        Utils::Log::Warn("Schema system init failed (button detection may not work).");

    Utils::Log::Info("Initializing entity system...");
    if (!Sdk::EntitySystem::Instance().Initialize())
        Utils::Log::Warn("Entity system init failed (menus may not work).");

    Utils::Log::Info("Resolving game event manager...");
    if (!Sdk::MessageSystem::Instance().InitGameEventManager())
        Utils::Log::Warn("Game event manager not resolved (center HTML display will not work).");

    Utils::Log::Info("Initializing ConVar service...");
    if (!Sdk::ConVarService::Instance().Initialize())
        Utils::Log::Warn("ConVar service init failed.");

    Utils::Log::Info("Initializing game event service...");
    if (!Sdk::GameEventService::Instance().Initialize())
        Utils::Log::Warn("Game event service init failed.");

    Utils::Log::Info("CS2Kit initialized.");
    return true;
}

void Shutdown()
{
    Core::Scheduler::Instance().CancelAll();
}

void OnGameFrame()
{
    Core::Scheduler::Instance().OnGameFrame();
    Menu::MenuManager::Instance().OnGameFrame();
}

void OnPlayerDisconnect(int slot)
{
    Menu::MenuManager::Instance().OnPlayerDisconnect(slot);
}

}  // namespace CS2Kit
