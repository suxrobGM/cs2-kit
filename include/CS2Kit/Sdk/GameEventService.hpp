#pragma once

#include <igameevents.h>

#include <CS2Kit/Core/CallbackRegistry.hpp>
#include <cstdint>
#include <functional>
#include <map>
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

    /**
     * @brief Attach listeners that the engine rejected at Listen() time. Called by the kit's
     * StartupServer hook on every map start.
     *
     * The event manager only knows an event's name after its definition file is loaded during
     * the first map startup, so AddListener calls made at plugin load (server still hibernating,
     * no map) fail and the listener would silently never fire.
     */
    void OnServerStartup();

    void FireGameEvent(IGameEvent* event) override;

private:
    struct RegisteredListener
    {
        std::string EventName;
        EventCallback Callback;
    };

    Core::CallbackRegistry<RegisteredListener> _listeners;
    std::map<std::string, bool> _registeredEvents;  // event name -> attached to the engine
};

}  // namespace CS2Kit::Sdk
