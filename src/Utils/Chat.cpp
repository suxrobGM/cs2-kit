#include <CS2Kit/Utils/Chat.hpp>

#include <CS2Kit/Players/PlayerManager.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/ChatColors.hpp>

#include <string>

namespace CS2Kit::Utils::Chat
{

namespace
{
// CS2 color escapes occupy bytes 0x01..0x10. If the message doesn't already start with
// one, we prepend the default color so the SayText2 renderer doesn't pick up whatever
// color was active in the previous chat line.
std::string EnsureColorPrefix(std::string_view message)
{
    if (!message.empty() && static_cast<unsigned char>(message.front()) <= 0x10)
        return std::string{message};

    std::string out;
    out.reserve(ChatColors::Default.size() + message.size());
    out.append(ChatColors::Default);
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
