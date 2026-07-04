#pragma once

#include <CS2Kit/Commands/Command.hpp>
#include <functional>
#include <unordered_map>
#include <vector>

namespace CS2Kit::Commands
{

/**
 * @brief Dispatches chat commands (prefixed with ! or .) to registered handlers.
 *
 * Permission checks go through `Engine().Policy.HasPermission`, and result/error messages
 * are delivered via `Engine().Policy.Reply` - set the policy once in OnLoad and every
 * command picks it up. No per-manager callback wiring.
 */
class CommandManager
{
public:
    CommandManager() = default;

    void Register(Command cmd);
    void Unregister(const std::string& name);
    bool HandleChatMessage(Players::Player* caller, const std::string& message);
    const Command* GetCommand(const std::string& name) const;
    std::vector<const Command*> GetAllCommands() const;

    void SetPrefixes(const std::vector<std::string>& prefixes) { _prefixes = prefixes; }

private:
    std::vector<std::string> ParseArguments(const std::string& text) const;

    std::unordered_map<std::string, Command> _commands;
    std::vector<std::string> _prefixes{"!", "."};
};

}  // namespace CS2Kit::Commands
