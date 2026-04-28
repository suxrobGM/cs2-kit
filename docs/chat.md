# Chat Output {#chat_guide}

[TOC]

> **Work in Progress** — The chat API may change.

## Overview

The chat module (`CS2Kit::Utils::Chat` and `CS2Kit::Utils::ChatColors`) is a thin layer over the engine's `TextMsg / HUD_PRINTTALK` user message (CS2 silently drops `SayText2` from non-player sources after the 2026 update). It gives plugins:

- **ChatColors** — `inline constexpr` constants for the 15 in-line CS2 chat color escapes, plus `ParseNamed` and `Strip` helpers
- **Chat::Print / PrintAll / PrintFiltered** — one-liners for sending colored chat lines to a slot, all players, or a filtered subset

Built on top of `CS2Kit::Sdk::MessageSystem::SendChatMessage`, which `CS2Kit::Initialize` wires up automatically — no extra setup needed.

## Color Constants

CS2's chat (`TextMsg / HUD_PRINTTALK`) reads ASCII bytes `0x01`-`0x10` as in-line color toggles. Embed them anywhere in a message; everything until the next escape (or `Default`) renders in that color. Byte values mirror the [SwiftlyS2 mapping](https://github.com/swiftly-solution/swiftlys2) — what CS2 actually renders today.

| Constant(s) | Byte | Color |
|---|---|---|
| `Default` / `White` | `\x01` | White (default) |
| `DarkRed` | `\x02` | Dark red |
| `LightPurple` | `\x03` | Light purple |
| `Green` | `\x04` | Green |
| `Olive` | `\x05` | Olive / dark green |
| `Lime` | `\x06` | Lime / light green |
| `Red` | `\x07` | Red |
| `Gray` / `Grey` | `\x08` | Gray |
| `Yellow` / `LightYellow` | `\x09` | Yellow |
| `Silver` / `BlueGrey` | `\x0A` | Silver / blue-grey |
| `LightBlue` / `Blue` | `\x0B` | Light blue |
| `DarkBlue` | `\x0C` | Dark blue |
| `Purple` / `Magenta` | `\x0E` | Purple / magenta |
| `LightRed` | `\x0F` | Light red |
| `Gold` / `Orange` | `\x10` | Gold / orange |

All are `inline constexpr std::string_view` so they compose cleanly with `std::format`. Names that share a byte (e.g. `Gold` / `Orange`) are aliases — pick whichever reads better at the call site.

## Sending Messages

```cpp
#include <CS2Kit/Utils/Chat.hpp>
#include <CS2Kit/Utils/ChatColors.hpp>

using namespace CS2Kit::Utils;

// One player:
Chat::Print(slot, "Welcome!");

// Everyone connected:
Chat::PrintAll("Server restarting in 5 minutes.");

// Subset selected by predicate:
Chat::PrintFiltered("Admin notice.", [](const Players::Player* p) {
    return AdminManager::Instance().IsAdmin(p->GetSteamID());
});
```

Bots and disconnected slots are skipped automatically. If `message` doesn't already start with a color escape, `Print` and the broadcast helpers prepend `ChatColors::Default` so the line renders cleanly regardless of whatever color was active in the previous chat line.

## Composing Colored Text

`std::format` is the natural fit:

```cpp
auto line = std::format(
    "{}[ADMIN]{} {}{}{} kicked {} for {}{}",
    ChatColors::Red,    ChatColors::Default,
    ChatColors::LightBlue, adminName, ChatColors::Default,
    targetName,
    ChatColors::Olive, reason);

Chat::PrintAll(line);
```

Renders as `[ADMIN]` in red, the admin name in light blue, the reason in olive, and everything else in the default color.

For runtime/config-driven colors, look up the escape by name:

```cpp
std::string_view color = ChatColors::ParseNamed(group.PrefixColor);
auto line = std::format("{}{} {}: {}", color, group.Prefix,
                        ChatColors::Default, message);
```

`ParseNamed` is case-insensitive and accepts every name in the table above (e.g. `"red"`, `"green"`, `"lightblue"`, `"silver"`, `"orange"`, `"grey"`). Aliases resolve to the same byte as their primary (`"orange"` → `Gold`, `"grey"` → `Gray`, `"magenta"` → `Purple`). Unknown names return `Default`.

## Stripping Colors for Logs

The engine renders `\x01`-`\x10` as colors, but writing them to a console or log file leaves garbage. Strip before logging:

```cpp
auto colored = std::format("{}[ADMIN]{} {}", ChatColors::Red,
                           ChatColors::Default, msg);
Chat::PrintAll(colored);
Log::Info("{}", ChatColors::Strip(colored));
```

## API Reference

### `Chat::`

| Function | Description |
|---|---|
| `Print(slot, message)` | Send a colored chat line to one player. |
| `PrintAll(message)` | Broadcast to every connected human player. Bots/disconnected slots skipped. |
| `PrintFiltered(message, filter)` | Broadcast only to players where `filter(player)` returns `true`. |

All accept `std::string_view`.

### `ChatColors::`

| Function | Returns | Description |
|---|---|---|
| `ParseNamed(name)` | `std::string_view` | Map a color name (`"red"`, `"green"`, ...) to its escape sequence. Case-insensitive; unknown names return `Default`. |
| `Strip(text)` | `std::string` | Remove all `0x01`-`0x10` escape bytes for safe console/log output. |

## Threading

Like the rest of CS2-Kit, the chat helpers are main-thread-only. Call them from your Metamod hooks, scheduled timers, or command handlers — never from a worker thread.
