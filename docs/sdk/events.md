# ConVars & Game Events {#sdk_events_guide}

[TOC]

## ConVarService

Read and write ConVars with type safety:

```cpp
auto& cvars = Engine().ConVars;

// Typed getters return std::optional
if (auto gravity = cvars.GetFloat("sv_gravity"))
    use(*gravity);

// Typed setters
cvars.SetFloat("sv_gravity", 400.0f);

// Listen for any convar change; returns an id for RemoveChangeListener
uint64_t id = cvars.OnChange([](const char* name, const char* oldValue, const char* newValue) {
    // Handle convar change
});
```

## GameEventService

Create, fire, and listen for game events:

```cpp
auto& events = Engine().Events;

// Listen for player death; returns an id you can pass to RemoveListener.
uint64_t id = events.Listen("player_death", [](IGameEvent* event) {
    int attacker = event->GetInt("attacker");
    int victim = event->GetInt("userid");
    // Handle event
});
```
