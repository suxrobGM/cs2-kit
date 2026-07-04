#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Sdk/GameEventService.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Utils/Log.hpp>

using CS2Kit::Core::Engine;

namespace CS2Kit::Sdk
{

using namespace CS2Kit::Utils;

bool GameEventService::Initialize()
{
    if (!Engine().Interfaces.GameEventManager)
    {
        Log::Warn("GameEventService: IGameEventManager2 not available.");
        return false;
    }

    Log::Info("Game event service initialized.");
    return true;
}

IGameEvent* GameEventService::CreateEvent(const char* name)
{
    auto* mgr = Engine().Interfaces.GameEventManager;
    if (!mgr)
        return nullptr;

    return mgr->CreateEvent(name);
}

bool GameEventService::FireEvent(IGameEvent* event, bool dontBroadcast)
{
    auto* mgr = Engine().Interfaces.GameEventManager;
    if (!mgr || !event)
        return false;

    return mgr->FireEvent(event, dontBroadcast);
}

void GameEventService::FreeEvent(IGameEvent* event)
{
    auto* mgr = Engine().Interfaces.GameEventManager;
    if (mgr && event)
        mgr->FreeEvent(event);
}

uint64_t GameEventService::Listen(const char* eventName, EventCallback callback)
{
    auto* mgr = Engine().Interfaces.GameEventManager;
    if (!mgr)
        return 0;

    if (_registeredEvents.find(eventName) == _registeredEvents.end())
    {
        mgr->AddListener(this, eventName, true);
        _registeredEvents.insert(eventName);
    }

    return _listeners.Add({eventName, std::move(callback)});
}

void GameEventService::RemoveListener(uint64_t id)
{
    _listeners.Remove(id);
}

void GameEventService::RemoveAllListeners()
{
    if (auto* mgr = Engine().Interfaces.GameEventManager)
        mgr->RemoveListener(this);  // detaches this listener from every event in one call

    _registeredEvents.clear();
    _listeners.Clear();
}

void GameEventService::FireGameEvent(IGameEvent* event)
{
    if (!event)
        return;

    const char* eventName = event->GetName();
    if (!eventName)
        return;

    for (const auto& [id, listener] : _listeners.Items())
    {
        if (listener.EventName == eventName && listener.Callback)
        {
            listener.Callback(event);
        }
    }
}

}  // namespace CS2Kit::Sdk
