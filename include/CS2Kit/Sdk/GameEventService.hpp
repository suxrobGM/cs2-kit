#pragma once

#include <igameevents.h>

#include <CS2Kit/Core/CallbackRegistry.hpp>
#include <cstdint>
#include <functional>
#include <set>
#include <string>

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

    /** Typed listen: @p TEvent is one of @ref CS2Kit::Sdk::Events (carries Name + From).
     *  The handler receives the decoded struct; the raw-IGameEvent overload above stays as
     *  the escape hatch for unmodeled events. */
    template <class TEvent>
    uint64_t Listen(std::function<void(const TEvent&)> handler)
    {
        return Listen(TEvent::Name, [h = std::move(handler)](IGameEvent* e) {
            if (e)
                h(TEvent::From(*e));
        });
    }

    void RemoveListener(uint64_t id);

    /** @brief Remove all listeners and deregister from the engine. Called by CS2Kit::Shutdown() (avoids
     * double-registration on reload). */
    void RemoveAllListeners();

    void FireGameEvent(IGameEvent* event) override;

private:
    struct RegisteredListener
    {
        std::string EventName;
        EventCallback Callback;
    };

    Core::CallbackRegistry<RegisteredListener> _listeners;
    std::set<std::string> _registeredEvents;
};

}  // namespace CS2Kit::Sdk
