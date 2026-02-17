#pragma once

#include <CS2Kit/Core/Singleton.hpp>

#include <igameevents.h>

#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>

namespace CS2Kit::Sdk
{

/**
 * Wrapper for IGameEventManager2 providing event creation, firing, and listener registration.
 */
class GameEventService : public Core::Singleton<GameEventService>, public IGameEventListener2
{
public:
    explicit GameEventService(Token) {}

    bool Initialize();

    IGameEvent* CreateEvent(const char* name);
    bool FireEvent(IGameEvent* event, bool dontBroadcast = false);
    void FreeEvent(IGameEvent* event);

    using EventCallback = std::function<void(IGameEvent*)>;
    uint64_t Listen(const char* eventName, EventCallback callback);
    void RemoveListener(uint64_t id);
    void FireGameEvent(IGameEvent* event) override;

private:
    struct RegisteredListener
    {
        std::string EventName;
        EventCallback Callback;
    };

    std::unordered_map<uint64_t, RegisteredListener> _listeners;
    uint64_t _nextListenerId = 1;
    std::set<std::string> _registeredEvents;
};

}  // namespace CS2Kit::Sdk
