#include <CS2Kit/Players/PlayerManager.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/Chat.hpp>
#include <CS2Kit/Utils/ChatColors.hpp>
#include <string>

namespace CS2Kit::Utils::Chat
{

namespace
{

// CS2 strips leading color escapes until a non-color byte. Prepend a space to
// preserve a leading color, or Default to avoid color carryover from the prior line.
std::string EnsureColorPrefix(std::string_view message)
{
    std::string_view prefix =
        (!message.empty() && static_cast<unsigned char>(message.front()) <= 0x10) ? " " : ChatColors::Default;

    std::string out;
    out.reserve(prefix.size() + message.size());
    out.append(prefix);
    out.append(message);
    return out;
}
}  // namespace

void Print(int slot, std::string_view message)
{
    Sdk::MessageSystem::Instance().SendChatMessage(slot, EnsureColorPrefix(message));
}

void PrintAll(std::string_view message)
{
    auto rendered = EnsureColorPrefix(message);
    auto& mgr = Players::PlayerManager::Instance();
    for (auto* p : mgr.GetAllPlayers())
    {
        if (!p)
            continue;
        Sdk::MessageSystem::Instance().SendChatMessage(p->GetSlot(), rendered);
    }
}

void PrintFiltered(std::string_view message, const std::function<bool(const Players::Player*)>& filter)
{
    if (!filter)
        return;

    auto rendered = EnsureColorPrefix(message);
    auto& mgr = Players::PlayerManager::Instance();
    for (auto* p : mgr.GetAllPlayers())
    {
        if (!p || !filter(p))
            continue;
        Sdk::MessageSystem::Instance().SendChatMessage(p->GetSlot(), rendered);
    }
}

}  // namespace CS2Kit::Utils::Chat
