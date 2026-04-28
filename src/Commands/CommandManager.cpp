#include <CS2Kit/Commands/CommandManager.hpp>
#include <CS2Kit/Utils/StringUtils.hpp>

namespace CS2Kit::Commands
{

using namespace CS2Kit::Utils;

void CommandManager::Register(Command cmd)
{
    _commands[StringUtils::ToLower(cmd.Name)] = std::move(cmd);
}

void CommandManager::Unregister(const std::string& name)
{
    _commands.erase(StringUtils::ToLower(name));
}

bool CommandManager::HandleChatMessage(Players::Player* caller, const std::string& message)
{
    if (!caller || message.empty())
        return false;

    bool hasPrefix = false;
    size_t prefixLen = 0;
    for (const auto& prefix : _prefixes)
    {
        if (message.size() >= prefix.size() && message.compare(0, prefix.size(), prefix) == 0)
        {
            hasPrefix = true;
            prefixLen = prefix.size();
            break;
        }
    }

    if (!hasPrefix)
        return false;

    auto parts = ParseArguments(message.substr(prefixLen));
    if (parts.empty())
        return false;

    const std::string& cmdName = parts[0];
    std::vector<std::string> args(parts.begin() + 1, parts.end());

    const Command* cmd = GetCommand(cmdName);
    if (!cmd)
        return false;

    auto reportResult = [&](const CommandResult& result) {
        if (_resultCallback)
            _resultCallback(caller, *cmd, result);
    };

    if (static_cast<int>(args.size()) < cmd->MinArgs ||
        (cmd->MaxArgs != 99 && static_cast<int>(args.size()) > cmd->MaxArgs))
    {
        std::string msg = "Usage: " + (cmd->Usage.empty() ? cmd->Name : cmd->Usage);
        reportResult({false, msg});
        return true;
    }

    if (!cmd->Permission.empty())
    {
        if (_permissionCallback && !_permissionCallback(caller->GetSteamID(), cmd->Permission))
        {
            reportResult({false, "You do not have permission to use this command."});
            return true;
        }
    }

    if (cmd->Handler)
    {
        auto result = cmd->Handler(caller, args);
        reportResult(result);
    }

    return true;
}

const Command* CommandManager::GetCommand(const std::string& name) const
{
    std::string lowerName = StringUtils::ToLower(name);

    auto it = _commands.find(lowerName);
    if (it != _commands.end())
        return &it->second;

    for (const auto& [key, cmd] : _commands)
    {
        if (cmd.Matches(name))
            return &cmd;
    }

    return nullptr;
}

std::vector<const Command*> CommandManager::GetAllCommands() const
{
    std::vector<const Command*> commands;
    commands.reserve(_commands.size());

    for (const auto& [name, cmd] : _commands)
    {
        commands.push_back(&cmd);
    }

    return commands;
}

std::vector<std::string> CommandManager::ParseArguments(const std::string& text) const
{
    return StringUtils::Split(text, ' ');
}

}  // namespace CS2Kit::Commands
