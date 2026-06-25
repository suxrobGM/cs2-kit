# SDK Wrappers {#sdk_guide}

[TOC]

## Overview

The Sdk module (`CS2Kit::Sdk`) provides typed wrappers around HL2SDK interfaces, abstracting away raw pointer manipulation and engine-specific details.

## GameInterfaces

Centralized holder for all SDK interface pointers. Automatically populated by `CS2Kit::Initialize()` — no manual setup needed.

```cpp
#include <CS2Kit/Core/Services.hpp>
using CS2Kit::Core::Engine;

// After CS2Kit::Initialize(), all interfaces are available:
auto& gi = Engine().Interfaces;
auto* engine = gi.Engine;       // IVEngineServer2*
auto* cvar = gi.CVar;           // ICvar*
auto* schema = gi.SchemaSystem; // ISchemaSystem*
// ... etc.
```

All other Sdk classes read from this holder internally. The other examples on this page reach their
service the same way — `Engine().GameData`, `Engine().Entities`, `Engine().Schema()`, and so on.

## GameData (Signature Management)

CS2-Kit ships built-in gamedata (`gamedata/signatures.jsonc`) that is automatically loaded during `CS2Kit::Initialize()`. The gamedata contains engine signatures and offsets used internally by Entity, PlayerController, and UserMessage subsystems.

Consumer plugins can also use GameData for their own signatures:

```cpp
auto& gd = Engine().GameData;

// Find a raw signature address
void* addr = gd.FindSignature("CCSPlayerController_Kick");

// Resolve a signature with RIP-relative offset
void* resolved = gd.ResolveSignature("SomeFunction");
```

### signatures.jsonc Format

```json
{
    "CCSPlayerController_Kick": {
        "windows": {
            "pattern": "48 89 5C 24 ?? 57 48 83 EC 30",
            "offset": 0
        },
        "linux": {
            "pattern": "55 48 89 E5 41 54 49 89 FC",
            "offset": 0
        }
    }
}
```

## EntitySystem

Access player controllers and entity data:

```cpp
auto& es = Engine().Entities;

// Get the raw controller entity by slot
auto* controller = es.GetPlayerController(slot);

// Check if a slot is valid
bool valid = es.IsPlayerSlotValid(slot);

// Read player button state
uint64_t buttons = es.GetPlayerButtons(slot);
```

For typed operations on a player, construct a @ref CS2Kit::Sdk::PlayerController from the slot (see
below) rather than working with the raw `CEntityInstance*`.

## PlayerController

Typed wrapper around `CCSPlayerController` providing common operations. Construct it from a player
slot — it resolves the controller entity internally (check `IsValid()` if the slot may be empty):

```cpp
CS2Kit::Sdk::PlayerController player(slot);

int health = player.GetHealth();
int team = player.GetTeam();

player.Kick("Cheating");
player.Slay();
player.ChangeTeam(3);  // CT
player.Respawn();
```

### Visibility

`SetVisible` toggles transparency on the player pawn body. Weapons, gloves, and grenades stay visible — CS2 routes those through systems a server plugin can't reach (the client-side glow/render pipeline), and there is currently no known server-side path to hide them.

```cpp
player.SetVisible(false);          // body fully invisible (alpha = 0)
player.SetVisible(false, 0x80);    // body 50% transparent
player.SetVisible(true);           // restore opaque
```

### Observer mode

Read or force a player into a specific spectator mode:

```cpp
using CS2Kit::Sdk::ObserverMode_t;

if (player.GetObserverMode() != ObserverMode_t::Roaming)
    player.SetObserverMode(ObserverMode_t::Roaming);
```

### Player name

Read/write `m_iszPlayerName` on the controller. `SetPlayerName` writes into the 128-byte fixed buffer (truncating to 127 chars + NUL); it does **not** trigger a scoreboard re-sync on its own. Replication piggybacks on the next state-change broadcast, so pair it with `ChangeTeam` or similar if you need an immediate refresh:

```cpp
auto saved = player.GetPlayerName();
player.SetPlayerName("");        // hide on scoreboard
// ... later
player.SetPlayerName(saved);
```

## EntityRender

Mutate `m_nRenderMode` and `m_clrRender` on any `CBaseModelEntity`. Used internally by `PlayerController::SetVisible`, but exposed so plugins can hide/recolor any entity (props, dropped weapons, world objects).

```cpp
using namespace CS2Kit::Sdk;

SetEntityRender(prop, RenderMode_t::TransTexture, ColorInvisible);
SetEntityRender(prop, RenderMode_t::Normal, ColorOpaqueWhite);
```

`m_clrRender` is RGBA packed as `(A << 24) | (B << 16) | (G << 8) | R` — `ColorInvisible` (`0x00FFFFFF`) is white at zero alpha.

## SchemaService

Resolves entity field offsets at runtime using CS2's schema system. Results are cached:

```cpp
auto& schema = Engine().Schema();

// Get field offset
int32_t offset = schema.GetOffset("CCSPlayerPawn", "m_iHealth");

// Use with entity pointer
int health = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pawn) + offset);
```

## SigScanner

Low-level byte-pattern scanning for finding functions in memory:

```cpp
using namespace CS2Kit::Sdk;

// Find a pattern in a module
void* addr = SigScanner::FindPattern(
    moduleBase, moduleSize,
    "48 89 5C 24 ?? 57 48 83 EC 30"
);

// Resolve a RIP-relative address
void* target = SigScanner::ResolveRelativeAddress(addr, 3, 7);
```

Wildcard bytes are represented as `??` in pattern strings.

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

## MessageSystem

Send chat and center-HTML messages to players:

```cpp
auto& msg = Engine().Messages;

// Send a chat line to a specific player (prefer CS2Kit::Utils::Chat for colored output)
msg.SendChatMessage(slot, "Hello, player!");

// Send / clear center HTML
msg.SendCenterHtml(slot, "<b>Important Notice</b>");
msg.ClearCenterHtml(slot);
```

## ChatInputCapture

Per-slot pending-prompt registry that backs the menu system's free-text @ref CS2Kit::Menu::InputOption. Use it directly when you need a prompt outside of a menu (e.g. a chat command that asks the player to type a value as a follow-up).

```cpp
auto& capture = Engine().ChatInput;

capture.BeginCapture(slot, "Enter your nickname:",
    [](int s, std::string_view text) -> bool {
        if (text.size() > 32) return false;        // re-prompt
        StoreNickname(s, std::string(text));
        return true;                                // accept
    },
    /*timeoutMs=*/30000);
```

The validator returns `true` to accept the input (capture clears) or `false` to re-prompt the player. The capture auto-cancels after `timeoutMs` of no input.

### Plumbing the chat hook

Suppressing a chat broadcast has to happen in the `say` / `say_team` hook. With @ref CS2Kit::Core::MetamodPluginBase that hook is the base's, routed to your `OnPlayerChat` override — call @ref CS2Kit::Sdk::ChatInputCapture::TryConsume there and return `true` to supersede:

```cpp
bool MyPlugin::OnPlayerChat(Player* p, std::string_view message, bool team) override
{
    if (Engine().ChatInput.TryConsume(p->GetSlot(), message))
        return true;   // capture handled it; don't broadcast
    return false;      // fall through to normal chat handling
}
```

If no capture is pending for the slot, `TryConsume` returns `false`.

### API

| Method | Description |
| --- | --- |
| `BeginCapture(slot, prompt, callback, timeoutMs = 30000)` | Start waiting for `slot`'s next chat line. Replaces any previous pending prompt for the same slot. |
| `IsCapturing(slot)` | `true` if `slot` has a pending prompt. |
| `TryConsume(slot, text)` | Route a chat line to the active prompt. Returns `true` when the message was consumed. |
| `CancelCapture(slot)` | Drop the pending prompt without firing the callback. |
| `GetPrompt(slot)` | Returns the active prompt string (used by `MenuRenderer` to draw the overlay), or `nullptr`. |
| `OnPlayerDisconnect(slot)` | Lifecycle hook — called automatically by `CS2Kit::OnPlayerDisconnect`. |
