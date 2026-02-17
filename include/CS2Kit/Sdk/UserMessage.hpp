#pragma once

#include <CS2Kit/Core/Singleton.hpp>
#include <string>

class IGameEventListener2;
class INetworkMessageInternal;
class CPlayerSlot;

namespace CS2Kit::Sdk
{

constexpr int UmSayText2 = 118;

/**
 * @brief Message system for sending chat and center HTML messages to players.
 */
class MessageSystem : public Core::Singleton<MessageSystem>
{
public:
    explicit MessageSystem(Token) {}

    bool Initialize();
    bool InitGameEventManager();
    void SendCenterHtml(int slot, const std::string& html);
    void SendChatMessage(int slot, const std::string& message);
    void ClearCenterHtml(int slot);

private:
    using GetLegacyGameEventListenerFn = IGameEventListener2* (*)(CPlayerSlot slot);

    GetLegacyGameEventListenerFn _getLegacyListener = nullptr;
    INetworkMessageInternal* _sayText2Internal = nullptr;
};

}  // namespace CS2Kit::Sdk
