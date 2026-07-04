# ConVars & Game Events {#sdk_events_guide}

[TOC]

## GameEventService

Prefer the typed listeners: each struct in `CS2Kit::Events` (`Sdk/GameEvents.hpp`) carries the event name and decodes its fields for you. Available: `PlayerDeath`, `PlayerSpawn`, `PlayerHurt`, `PlayerTeam`, `PlayerConnectFull`, `WeaponFire`, `RoundStart`, `RoundEnd`, `RoundPrestart`.

```cpp
namespace Events = CS2Kit::Events;
auto& events = Engine().Events;

uint64_t id = events.Listen<Events::PlayerDeath>([](const Events::PlayerDeath& e) {
    // e.VictimSlot, e.AttackerSlot, e.Headshot, e.Weapon, ...
});

events.RemoveListener(id);   // or leave it - Shutdown removes everything on unload
```

For events the kit hasn't modeled, the stringly overload is the escape hatch - same registration, raw `IGameEvent*`:

```cpp
events.Listen("bomb_planted", [](IGameEvent* event) {
    int site = event->GetInt("site");
});
```

You can also create and fire events (`CreateEvent` / `FireEvent` / `FreeEvent`) - the center-HTML transport is built on exactly that.

## ConVarService

Typed reads and writes over ICvar:

```cpp
auto& cvars = Engine().ConVars;

if (auto gravity = cvars.GetFloat("sv_gravity"))   // getters return std::optional
    Use(*gravity);

cvars.SetFloat("sv_gravity", 400.0f);
cvars.ExecuteServerCommand("mp_restartgame 1");

// Global change listener; the id cancels it via RemoveChangeListener.
uint64_t id = cvars.OnChange([](const char* name, const char* oldValue, const char* newValue) {
    /* ... */
});
```
