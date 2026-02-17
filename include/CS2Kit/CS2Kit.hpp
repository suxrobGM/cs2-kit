#pragma once

#include <CS2Kit/Core/ILogger.hpp>
#include <eiface.h>
#include <icvar.h>

// Forward declarations for HL2SDK types
class IGameEventSystem;
class INetworkMessages;
class ISchemaSystem;
class IGameResourceService;

namespace CS2Kit
{

/**
 * @brief Initialization parameters for CS2Kit.
 * Consumers populate SDK interfaces (via Metamod's GET_V_IFACE macros)
 * and pass them here. CS2Kit stores them in GameInterfaces and initializes
 * all internal subsystems.
 */
struct InitParams
{
    // Server base directory (from ISmmAPI::GetBaseDir())
    const char* BaseDir = nullptr;

    // SDK interfaces (all required)
    IServerGameDLL* ServerGameDLL = nullptr;
    IServerGameClients* ServerGameClients = nullptr;
    IVEngineServer2* Engine = nullptr;
    IGameEventSystem* GameEventSystem = nullptr;
    INetworkMessages* NetworkMessages = nullptr;
    ISchemaSystem* SchemaSystem = nullptr;
    ICvar* CVar = nullptr;
    IGameResourceService* GameResourceService = nullptr;

    // Configuration
    const char* LogPrefix = "CS2Kit";    ///< Prefix for console log messages (e.g., "[AdminSystem]")
    const char* GameDataPath = nullptr;  ///< Path to signatures.jsonc (relative to server base dir)
    Core::ILogger* Logger = nullptr;     ///< Optional custom logger. If null, uses built-in ConsoleLogger.
};

/**
 * @brief Initialize all CS2Kit subsystems.
 * Call from Plugin::Load() after populating InitParams with SDK interfaces.
 * @return true on success, false if critical subsystems failed to initialize.
 */
bool Initialize(const InitParams& params);

/**
 * @brief Shut down all CS2Kit subsystems.
 * Call from Plugin::Unload().
 */
void Shutdown();

/**
 * @brief Process one game frame. Drives Scheduler and MenuManager.
 * Call from Hook_GameFrame().
 */
void OnGameFrame();

/**
 * @brief Clean up state when a player disconnects.
 * Call from Hook_ClientDisconnect().
 */
void OnPlayerDisconnect(int slot);

}  // namespace CS2Kit
