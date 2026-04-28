#include "Sdk/SigScanner.hpp"

#include <igameevents.h>

#include <CS2Kit/Sdk/GameData.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <engine/igameeventsystem.h>
#include <irecipientfilter.h>
#include <networksystem/inetworkmessages.h>
#include <networksystem/netmessage.h>
#include <usermessages.pb.h>

namespace CS2Kit::Sdk
{

using namespace CS2Kit::Utils;

bool MessageSystem::Initialize()
{
    auto& interfaces = GameInterfaces::Instance();

    if (!interfaces.GameEventSystem)
    {
        Log::Error("IGameEventSystem not available.");
        return false;
    }

    if (!interfaces.NetworkMessages)
    {
        Log::Error("INetworkMessages not available.");
        return false;
    }

    Log::Info("Message system initialized.");
    return true;
}

bool MessageSystem::InitGameEventManager()
{
    auto& interfaces = GameInterfaces::Instance();
    auto& gameData = GameData::Instance();

    void* eventManagerAddr = gameData.ResolveSignature("GameEventManager");
    if (eventManagerAddr)
    {
        interfaces.GameEventManager =
            *reinterpret_cast<IGameEventManager2**>(reinterpret_cast<uintptr_t>(eventManagerAddr));

        if (interfaces.GameEventManager)
        {
            Log::Info("Game event manager resolved at {:#x}.",
                      reinterpret_cast<uintptr_t>(interfaces.GameEventManager));
        }
        else
        {
            Log::Warn("Game event manager pointer is null after resolve.");
        }
    }
    else
    {
        Log::Warn("GameEventManager signature not found.");
    }

    void* legacyListenerAddr = gameData.FindSignature("LegacyGameEventListener");
    if (legacyListenerAddr)
    {
        _getLegacyListener = reinterpret_cast<GetLegacyGameEventListenerFn>(legacyListenerAddr);
        Log::Info("LegacyGameEventListener resolved.");
    }
    else
    {
        Log::Warn("LegacyGameEventListener signature not found (will use broadcast fallback).");
    }

    return interfaces.GameEventManager != nullptr;
}

void MessageSystem::SendCenterHtml(int slot, const std::string& html)
{
    auto* gameEventManager = GameInterfaces::Instance().GameEventManager;
    if (!gameEventManager || slot < 0 || slot >= 64)
        return;

    IGameEvent* pEvent = gameEventManager->CreateEvent("show_survival_respawn_status");
    if (!pEvent)
        return;

    pEvent->SetString("loc_token", html.c_str());
    pEvent->SetInt("userid", slot);
    pEvent->SetInt("duration", 5);

    if (_getLegacyListener)
    {
        IGameEventListener2* pListener = _getLegacyListener(CPlayerSlot(slot));
        if (pListener)
        {
            pListener->FireGameEvent(pEvent);
            gameEventManager->FreeEvent(pEvent);
            return;
        }
    }

    gameEventManager->FireEvent(pEvent);
}

void MessageSystem::SendChatMessage(int slot, const std::string& message)
{
    auto& interfaces = GameInterfaces::Instance();
    if (!interfaces.GameEventSystem || !interfaces.NetworkMessages || slot < 0 || slot >= 64)
        return;

    // CS2 routes server-originated chat through TextMsg with dest=HUD_PRINTTALK rather than
    // SayText2. SayText2 requires a real source player and silently drops messages whose
    // entityindex doesn't resolve to a connected client.
    if (!_textMsgInternal)
    {
        _textMsgInternal = interfaces.NetworkMessages->FindNetworkMessage("CUserMessageTextMsg");
        if (!_textMsgInternal)
            _textMsgInternal = interfaces.NetworkMessages->FindNetworkMessagePartial("TextMsg");
    }

    if (!_textMsgInternal)
        return;

    CNetMessage* pMsg = _textMsgInternal->AllocateMessage();
    if (!pMsg)
        return;

    auto* pTextMsg = pMsg->ToPB<CUserMessageTextMsg>();
    if (!pTextMsg)
    {
        interfaces.NetworkMessages->DeallocateNetMessageAbstract(_textMsgInternal, pMsg);
        return;
    }
    pTextMsg->set_dest(HudPrintTalk);
    pTextMsg->add_param(message.c_str());

    uint64_t clients = (1ULL << slot);

    interfaces.GameEventSystem->PostEventAbstract(-1, false, 1, &clients, _textMsgInternal, pMsg, 0,
                                                  NetChannelBufType_t::BUF_RELIABLE);

    interfaces.NetworkMessages->DeallocateNetMessageAbstract(_textMsgInternal, pMsg);
}

void MessageSystem::ClearCenterHtml(int slot)
{
    SendCenterHtml(slot, " ");
}

}  // namespace CS2Kit::Sdk
