#include <CS2Kit/Sdk/GameEventService.hpp>

#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Utils/Log.hpp>

namespace CS2Kit::Sdk
{

using namespace CS2Kit::Utils;

bool GameEventService::Initialize()
{
    if (!GameInterfaces::Instance().GameEventManager)
    {
        Log::Warn("GameEventService: IGameEventManager2 not available.");
        return false;
    }

    Log::Info("Game event service initialized.");
    return true;
}

IGameEvent* GameEventService::CreateEvent(const char* name)
{
    auto* mgr = GameInterfaces::Instance().GameEventManager;
    if (!mgr)
        return nullptr;

    return mgr->CreateEvent(name);
}

bool GameEventService::FireEvent(IGameEvent* event, bool dontBroadcast)
{
    auto* mgr = GameInterfaces::Instance().GameEventManager;
    if (!mgr || !event)
        return false;

    return mgr->FireEvent(event, dontBroadcast);
}

void GameEventService::FreeEvent(IGameEvent* event)
{
    auto* mgr = GameInterfaces::Instance().GameEventManager;
    if (mgr && event)
        mgr->FreeEvent(event);
}

uint64_t GameEventService::Listen(const char* eventName, EventCallback callback)
{
    auto* mgr = GameInterfaces::Instance().GameEventManager;
    if (!mgr)
        return 0;

    if (_registeredEvents.find(eventName) == _registeredEvents.end())
    {
        mgr->AddListener(this, eventName, true);
        _registeredEvents.insert(eventName);
    }

    uint64_t id = _nextListenerId++;
    _listeners[id] = {eventName, std::move(callback)};
    return id;
}

void GameEventService::RemoveListener(uint64_t id)
{
    _listeners.erase(id);
}

void GameEventService::FireGameEvent(IGameEvent* event)
{
    if (!event)
        return;

    const char* eventName = event->GetName();
    if (!eventName)
        return;

    for (auto& [id, listener] : _listeners)
    {
        if (listener.EventName == eventName && listener.Callback)
        {
            listener.Callback(event);
        }
    }
}

}  // namespace CS2Kit::Sdk
