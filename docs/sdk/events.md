# ConVars & Game Events {#sdk_events_guide}

[TOC]

## GameEventService

Prefer the typed listeners: each struct in `CS2Kit::Events` (`Sdk/GameEvents.hpp`) carries the event name and decodes its fields for you. Available: `PlayerDeath`, `PlayerSpawn`, `PlayerJump`, `PlayerHurt`, `PlayerTeam`, `PlayerConnectFull`, `WeaponFire`, `RoundStart`, `RoundEnd`, `RoundPrestart`.

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

The setters go through the engine: change callbacks fire and `FCVAR_REPLICATED` values broadcast to every client. That is what you want for server-wide changes; two escape hatches cover the per-player cases.

### Per-client replication

@ref CS2Kit::Sdk::ConVarService::ReplicateToClient "ReplicateToClient" sends `CNETMsg_SetConVar` to a single client, so only that client's view of a replicated convar changes - the server value and every other client are untouched. This is how you make *one* player's prediction run with different movement settings (the bhop plugin replicates `sv_autobunnyhopping` to granted players):

```cpp
cvars.ReplicateToClient(slot, "sv_autobunnyhopping", "1");
```

The client's connect/map-change snapshot restores the server value, so re-send the override from a `PlayerSpawn` listener to keep it sticky.

### Raw value access

@ref CS2Kit::Sdk::RawConVar "Raw(name)" returns a handle to the convar's raw storage: reads and writes skip change callbacks *and* replication. Use it for scoped flips around one player's processing (e.g. inside a @ref CS2Kit::Sdk::MovementHook "MovementHook" pre/post pair), where the engine setters' broadcast would leak the change to everyone. You are responsible for restoring the prior value; the handle stays valid for the convar's lifetime, so resolve once and cache.

```cpp
CS2Kit::RawConVar autoBhop = cvars.Raw("sv_autobunnyhopping");
bool saved = autoBhop.GetBool();
autoBhop.SetBool(true);   // no callbacks, nothing networked
// ... run the per-player work ...
autoBhop.SetBool(saved);
```
