#include "Core/ConsoleLogger.hpp"
#include "Sdk/Schema.hpp"

#include <CS2Kit/CS2Kit.hpp>
#include <CS2Kit/Core/ILogger.hpp>
#include <CS2Kit/Core/Paths.hpp>
#include <CS2Kit/Core/Scheduler.hpp>
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Menu/MenuManager.hpp>
#include <CS2Kit/Sdk/ChatInputCapture.hpp>
#include <CS2Kit/Sdk/ConVarService.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/EntityOps.hpp>
#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/GameEventService.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Sdk/PrecacheService.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <ISmmAPI.h>
#include <eiface.h>
#include <engine/igameeventsystem.h>
#include <format>
#include <icvar.h>
#include <interfaces/interfaces.h>
#include <networksystem/inetworkmessages.h>
#include <schemasystem/schemasystem.h>

namespace CS2Kit
{

static constexpr const char* DefaultGameDataPath = "addons/cs2-kit/gamedata/signatures.jsonc";
static Core::ConsoleLogger g_consoleLogger;

bool Initialize(ISmmAPI* ismm, char* error, size_t maxlen, Core::Services& services, const InitParams& params)
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

    auto& gi = services.Interfaces;

    // Resolve each required interface, erroring out on the first one that is missing. The macro
    // keeps this type-safe (decltype, no void** punning) while collapsing the per-interface
    // resolve-and-check boilerplate to one line each.
#define CS2KIT_RESOLVE(field, factory, version)                               \
    gi.field = static_cast<decltype(gi.field)>(factory(version));             \
    if (!gi.field)                                                            \
    {                                                                         \
        ismm->Format(error, maxlen, "Could not find interface: %s", version); \
        return false;                                                         \
    }

    CS2KIT_RESOLVE(ServerGameDLL, resolveServer, INTERFACEVERSION_SERVERGAMEDLL)
    CS2KIT_RESOLVE(ServerGameClients, resolveServer, INTERFACEVERSION_SERVERGAMECLIENTS)
    CS2KIT_RESOLVE(GameEntities, resolveServer, INTERFACEVERSION_SERVERGAMEENTS)
    CS2KIT_RESOLVE(Engine, resolveEngine, INTERFACEVERSION_VENGINESERVER)
    CS2KIT_RESOLVE(GameEventSystem, resolveEngine, GAMEEVENTSYSTEM_INTERFACE_VERSION)
    CS2KIT_RESOLVE(NetworkMessages, resolveEngine, NETWORKMESSAGES_INTERFACE_VERSION)
    CS2KIT_RESOLVE(SchemaSystem, resolveEngine, SCHEMASYSTEM_INTERFACE_VERSION)
    CS2KIT_RESOLVE(CVar, resolveEngine, CVAR_INTERFACE_VERSION)
    CS2KIT_RESOLVE(GameResourceService, resolveEngine, GAMERESOURCESERVICESERVER_INTERFACE_VERSION)

#undef CS2KIT_RESOLVE

    // 4. Set g_pCVar (required by HL2SDK's tier1/convar.cpp)
    g_pCVar = gi.CVar;

    // 5. Load game data (signatures and offsets)
    const char* gameDataPath = params.GameDataPath ? params.GameDataPath : DefaultGameDataPath;
    Utils::Log::Info("Loading game data from {}...", gameDataPath);
    services.GameData.Load(gameDataPath);

    // 6. Initialize SDK subsystems
    Utils::Log::Info("Initializing SDK message system...");
    if (!services.Messages.Initialize())
    {
        Utils::Log::Error("Failed to initialize message system.");
        return false;
    }

    Utils::Log::Info("Initializing schema system...");
    if (!services.Schema().Initialize())
        Utils::Log::Warn("Schema system init failed (button detection may not work).");

    Utils::Log::Info("Initializing entity system...");
    if (!services.Entities.Initialize())
        Utils::Log::Warn("Entity system init failed (menus may not work).");

    Utils::Log::Info("Initializing entity ops...");
    if (!services.EntityOps.Initialize())
        Utils::Log::Warn("Entity ops unavailable (spawned effects degrade; see signature warnings above).");

    Utils::Log::Info("Registering precache game system...");
    if (!services.Precache.Initialize(std::format("{}_CS2KitPrecache", params.LogPrefix)))
        Utils::Log::Warn("Precache game system not registered (resource precaching unavailable).");

    Utils::Log::Info("Resolving game event manager...");
    if (!services.Messages.InitGameEventManager())
        Utils::Log::Warn("Game event manager not resolved (center HTML display will not work).");

    Utils::Log::Info("Initializing ConVar service...");
    if (!services.ConVars.Initialize())
        Utils::Log::Warn("ConVar service init failed.");

    Utils::Log::Info("Initializing game event service...");
    if (!services.Events.Initialize())
        Utils::Log::Warn("Game event service init failed.");

    Utils::Log::Info("Initializing transmit filter...");
    if (!services.Transmit.Initialize())
        Utils::Log::Warn("Transmit filter inert (CheckTransmitPlayerSlot offset missing from gamedata).");

    // Per-frame subsystems pump through the scheduler (PostgresDatabase registers its own pump
    // in Start), so OnGameFrame has exactly one thing to tick. CancelAll in Shutdown unhooks
    // these; Initialize re-registers them on the next load.
    services.Scheduler.EveryFrame([&services] { services.Menus.OnGameFrame(); });
    services.Scheduler.EveryFrame([&services] { services.Http.DispatchCompletions(); });

    Utils::Log::Info("CS2Kit initialized.");
    return true;
}

void Shutdown(Core::Services& services)
{
    services.Precache.Shutdown();  // first: the engine must stop referencing our vtables
    services.Events.RemoveAllListeners();
    services.Http.Stop();  // drains in-flight requests before their completion targets go away
    services.Scheduler.CancelAll();
}

void OnGameFrame(Core::Services& services)
{
    services.Scheduler.OnGameFrame();
}

void OnPlayerDisconnect(Core::Services& services, int slot)
{
    services.Menus.OnPlayerDisconnect(slot);
    services.ChatInput.OnPlayerDisconnect(slot);
    services.Transmit.OnPlayerDisconnect(slot);
}

}  // namespace CS2Kit
