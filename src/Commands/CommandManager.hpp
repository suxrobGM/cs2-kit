#pragma once

#include "../Core/Singleton.hpp"
#include "Command.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

namespace CS2Kit::Commands
{

/**
 * Permission check callback type.
 * @param steamId  Caller's SteamID (0 for console).
 * @param permission Required permission flags (e.g., "c" for kick, "d" for ban).
 * @return True if the caller has the required permission.
 */
using PermissionCallback = std::function<bool(int64_t steamId, const std::string& permission)>;

/**
 * Dispatches chat commands (prefixed with ! or .) to registered handlers.
 * Handles prefix matching, argument parsing, and permission enforcement.
 */
class CommandManager : public Core::Singleton<CommandManager>
{
public:
    explicit CommandManager(Token) { _prefixes = {"!", "."}; }

    void Register(Command cmd);
    void Unregister(const std::string& name);
    bool HandleChatMessage(ICommandCaller* caller, const std::string& message);
    const Command* GetCommand(const std::string& name) const;
    std::vector<const Command*> GetAllCommands() const;

    void SetPrefixes(const std::vector<std::string>& prefixes) { _prefixes = prefixes; }
    void SetPermissionCallback(PermissionCallback callback) { _permissionCallback = std::move(callback); }

private:
    std::vector<std::string> ParseArguments(const std::string& text) const;

    std::unordered_map<std::string, Command> _commands;
    std::vector<std::string> _prefixes;
    PermissionCallback _permissionCallback;
};

}  // namespace CS2Kit::Commands
