#pragma once

#include <CS2Kit/Commands/Command.hpp>
#include <CS2Kit/Core/Singleton.hpp>
#include <functional>
#include <unordered_map>
#include <vector>

namespace CS2Kit::Commands
{

/**
 * @brief check callback type.
 *
 * @param steamId  Caller's SteamID (0 for console).
 * @param permission Required permission flags (e.g., "c" for kick, "d" for ban).
 * @return True if the caller has the required permission.
 */
using PermissionCallback = std::function<bool(int64_t steamId, const std::string& permission)>;

/**
 * @brief Invoked after a command handler returns.
 *
 * Lets the host plugin pipe `CommandResult.Message` (success or error) back to the caller —
 * e.g., as a colored chat reply. Also invoked for early dispatch failures (bad arg count,
 * permission denied) with a synthesized `CommandResult` so the caller still gets feedback.
 */
using ResultCallback = std::function<void(Players::Player* caller, const Command& cmd, const CommandResult& result)>;

/**
 * @brief Dispatches chat commands (prefixed with ! or .) to registered handlers.
 * Handles prefix matching, argument parsing, and permission enforcement.
 */
class CommandManager : public Core::Singleton<CommandManager>
{
public:
    explicit CommandManager(Token) { _prefixes = {"!", "."}; }

    void Register(Command cmd);
    void Unregister(const std::string& name);
    bool HandleChatMessage(Players::Player* caller, const std::string& message);
    const Command* GetCommand(const std::string& name) const;
    std::vector<const Command*> GetAllCommands() const;

    void SetPrefixes(const std::vector<std::string>& prefixes) { _prefixes = prefixes; }
    void SetPermissionCallback(PermissionCallback callback) { _permissionCallback = std::move(callback); }
    void SetResultCallback(ResultCallback callback) { _resultCallback = std::move(callback); }

private:
    std::vector<std::string> ParseArguments(const std::string& text) const;

    std::unordered_map<std::string, Command> _commands;
    std::vector<std::string> _prefixes;
    PermissionCallback _permissionCallback;
    ResultCallback _resultCallback;
};

}  // namespace CS2Kit::Commands
