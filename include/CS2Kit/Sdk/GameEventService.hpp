#pragma once

#include <igameevents.h>

#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>

namespace CS2Kit::Sdk
{

/**
 * @brief Wrapper for IGameEventManager2 providing event creation, firing, and listener registration.
 */
class GameEventService : public IGameEventListener2
{
public:
    GameEventService() = default;

    bool Initialize();

    IGameEvent* CreateEvent(const char* name);
    bool FireEvent(IGameEvent* event, bool dontBroadcast = false);
    void FreeEvent(IGameEvent* event);

    using EventCallback = std::function<void(IGameEvent*)>;
    uint64_t Listen(const char* eventName, EventCallback callback);
    void RemoveListener(uint64_t id);

    /** @brief Remove all listeners and deregister from the engine. Called by CS2Kit::Shutdown() (avoids double-registration on reload). */
    void RemoveAllListeners();

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
