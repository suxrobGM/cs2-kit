# SDK Wrappers {#sdk_guide}

[TOC]

> **Work in Progress** — The SDK wrapper API may change.

## Overview

The Sdk module (`CS2Kit::Sdk`) provides typed wrappers around HL2SDK interfaces, abstracting away raw pointer manipulation and engine-specific details.

## GameInterfaces

Centralized holder for all SDK interface pointers. Populate during `Plugin::Load()`:

```cpp
auto& gi = CS2Kit::Sdk::GameInterfaces::Instance();
gi.Engine = ...;       // IVEngineServer2*
gi.CVar = ...;         // ICvar*
gi.SchemaSystem = ...; // ISchemaSystem*
// ... etc.
```

All other Sdk classes read from this singleton internally.

## GameData (Signature Management)

Loads a JSON file mapping signature names to platform-specific byte patterns and offsets:

```cpp
auto& gd = CS2Kit::Sdk::GameData::Instance();
gd.Load("gamedata/signatures.jsonc");

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
auto& es = CS2Kit::Sdk::EntitySystem::Instance();

// Get player controller by slot
auto* controller = es.GetPlayerController(slot);

// Check if a slot is valid
bool valid = es.IsPlayerSlotValid(slot);

// Read player button state
uint64_t buttons = es.GetPlayerButtons(slot);
```

## PlayerController

Typed wrapper around `CCSPlayerController` providing common operations:

```cpp
CS2Kit::Sdk::PlayerController player(controllerPtr);

int health = player.GetHealth();
int team = player.GetTeam();

player.Kick("Cheating");
player.Slay();
player.ChangeTeam(3);  // CT
player.Respawn();
```

## SchemaService

Resolves entity field offsets at runtime using CS2's schema system. Results are cached:

```cpp
auto& schema = CS2Kit::Sdk::SchemaService::Instance();

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
auto& cvars = CS2Kit::Sdk::ConVarService::Instance();

// Read a convar value
auto value = cvars.GetValue<float>("sv_gravity");

// Set a convar value
cvars.SetValue("sv_gravity", 400.0f);

// Listen for changes
cvars.OnChange("sv_cheats", [](const std::string& name) {
    // Handle convar change
});
```

## GameEventService

Create, fire, and listen for game events:

```cpp
auto& events = CS2Kit::Sdk::GameEventService::Instance();

// Listen for player death
events.AddListener("player_death", [](IGameEvent* event) {
    int attacker = event->GetInt("attacker");
    int victim = event->GetInt("userid");
    // Handle event
});
```

## MessageSystem

Send chat and center-HTML messages to players:

```cpp
auto& msg = CS2Kit::Sdk::MessageSystem::Instance();

// Send chat message to a specific player
msg.SendChat(slot, "Hello, player!");

// Send center HTML to a player
msg.SendCenterHtml(slot, "<b>Important Notice</b>");

// Clear center HTML
msg.ClearCenterHtml(slot);
```
