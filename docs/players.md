# Player Management {#players_guide}

[TOC]

## Overview

The player module (`CS2Kit::Players`) tracks connected players by slot and SteamID. It is intentionally minimal — only identity and connection metadata. Plugin-specific state (admin flags, punishment cache, stats) belongs in your own managers, keyed off SteamID, not on the `Player` type.

- **Player** — Identity record: slot, SteamID, name, IP, connect time
- **PlayerManager** — Singleton with two indexed maps (slot, SteamID) for O(1) lookup

Single-threaded by design. All access happens from game-thread Metamod hooks, so no mutex is needed.

## Wiring Up

If your plugin derives from @ref CS2Kit::Core::MetamodPluginBase, this is automatic: the base calls `AddPlayer` on connect and `RemovePlayer` on disconnect, then hands your `OnPlayerConnect(Player*)` / `OnPlayerDisconnect(Player*)` overrides the live `Player*`. Just override those.

Without the base, drive `PlayerManager` from your own connect/disconnect hooks:

```cpp
// OnClientConnected hook:
PlayerManager::Instance().AddPlayer(slot.Get(), static_cast<int64_t>(xuid),
                                    name ? name : "", address ? address : "");

// ClientDisconnect hook:
CS2Kit::OnPlayerDisconnect(slot.Get());
PlayerManager::Instance().RemovePlayer(slot.Get());

// Plugin::Unload():
PlayerManager::Instance().Clear();
```

## Player API

| Method | Returns | Description |
|--------|---------|-------------|
| `GetSlot()` | `int` | Player's slot index (0–63) |
| `GetSteamID()` | `int64_t` | 64-bit SteamID |
| `GetName()` | `const std::string&` | Display name at connect time (use `SetName` to refresh on rename) |
| `GetIpAddress()` | `const std::string&` | Connection IP |
| `GetConnectTime()` | `int64_t` | Unix timestamp when the player joined |
| `GetPlaytime()` | `int64_t` | Seconds since `GetConnectTime` |
| `SetName(name)` | `void` | Update the cached name |

## PlayerManager API

| Method | Returns | Description |
|--------|---------|-------------|
| `AddPlayer(slot, steamId, name, ip)` | `Player*` | Create or replace the player in this slot. Returned pointer is owned by the manager. |
| `RemovePlayer(slot)` | `void` | Drop the player from both indexes. |
| `Clear()` | `void` | Drop all players (call from `Plugin::Unload`). |
| `GetPlayerBySlot(slot)` | `Player*` or `nullptr` | O(1) slot lookup. |
| `GetPlayerBySteamId(steamId)` | `Player*` or `nullptr` | O(1) SteamID lookup. |
| `FindPlayersByName(substring)` | `std::vector<Player*>` | Case-insensitive `contains` match across all players. |
| `GetAllPlayers()` | `std::vector<Player*>` | Snapshot of every connected player. |
| `GetPlayerCount()` | `size_t` | Count of currently tracked players. |

## Pointer Lifetime

`Player*` returned by any of the lookup methods is owned by `PlayerManager` and remains valid until:

- `RemovePlayer(slot)` is called for that slot, or
- `AddPlayer(slot, …)` reassigns that slot (e.g., a new player joins the same slot), or
- `Clear()` is called.

Do not store `Player*` across the disconnect callback. If you need to remember a player after they leave, copy the SteamID — that is the stable identifier.

## Plugin-Specific State

`Player` deliberately does not carry admin flags, mute/gag flags, or any other plugin concern. Keep that state in your own services, keyed by SteamID:

```cpp
class AdminManager : public CS2Kit::Core::Singleton<AdminManager>
{
public:
    explicit AdminManager(Token) {}
    bool IsAdmin(int64_t steamId) const;
    // ...
};

// Query when you need it:
auto* player = PlayerManager::Instance().GetPlayerBySlot(slot);
if (player && AdminManager::Instance().IsAdmin(player->GetSteamID()))
{
    // ...
}
```

This keeps `CS2Kit::Players::Player` reusable across plugins that have nothing to do with admins or punishments.
