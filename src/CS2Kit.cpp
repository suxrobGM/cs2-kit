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

namespace CS2Kit
{

static Core::ConsoleLogger g_consoleLogger;

bool Initialize(const InitParams& params)
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
    if (params.BaseDir)
        Core::SetBaseDir(params.BaseDir);

    Utils::Log::Info("Initializing CS2Kit...");

    // 3. Populate GameInterfaces singleton
    auto& gi = Sdk::GameInterfaces::Instance();
    gi.ServerGameDLL = params.ServerGameDLL;
    gi.ServerGameClients = params.ServerGameClients;
    gi.Engine = params.Engine;
    gi.GameEventSystem = params.GameEventSystem;
    gi.NetworkMessages = params.NetworkMessages;
    gi.SchemaSystem = params.SchemaSystem;
    gi.CVar = params.CVar;
    gi.GameResourceService = params.GameResourceService;

    // 4. Load game data (signatures and offsets)
    if (params.GameDataPath)
    {
        Utils::Log::Info("Loading game data...");
        Sdk::GameData::Instance().Load(params.GameDataPath);
    }

    // 5. Initialize SDK subsystems
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
