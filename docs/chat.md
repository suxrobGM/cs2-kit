# Chat Output {#chat_guide}

[TOC]

> **Work in Progress** — The chat API may change.

## Overview

The chat module (`CS2Kit::Utils::Chat` and `CS2Kit::Utils::ChatColors`) is a thin layer over the engine's SayText2 user message. It gives plugins:

- **ChatColors** — `inline constexpr` constants for the 15 in-line CS2 chat color escapes, plus `ParseNamed` and `Strip` helpers
- **Chat::Print / PrintAll / PrintFiltered** — one-liners for sending colored chat lines to a slot, all players, or a filtered subset

Built on top of `CS2Kit::Sdk::MessageSystem::SendChatMessage`, which `CS2Kit::Initialize` wires up automatically — no extra setup needed.

## Color Constants

CS2 SayText2 reads ASCII bytes `0x01`-`0x10` as in-line color toggles. Embed them anywhere in a message; everything until the next escape (or `Default`) renders in that color.

| Constant | Byte | Color |
|---|---|---|
| `ChatColors::Default` | `\x01` | Default white/yellow |
| `ChatColors::Red` | `\x02` | Red |
| `ChatColors::LightPurple` | `\x03` | Light purple |
| `ChatColors::Green` | `\x04` | Green |
| `ChatColors::Olive` | `\x05` | Olive / dark green |
| `ChatColors::Lime` | `\x06` | Lime / light green |
| `ChatColors::LightRed` | `\x07` | Light red / silver |
| `ChatColors::Gray` | `\x08` | Gray |
| `ChatColors::LightYellow` | `\x09` | Light yellow |
| `ChatColors::LightBlue` | `\x0A` | Light blue |
| `ChatColors::Blue` | `\x0B` | Blue |
| `ChatColors::Purple` | `\x0C` | Purple |
| `ChatColors::Pink` | `\x0D` | Pink |
| `ChatColors::Gold` | `\x0E` | Gold / orange |
| `ChatColors::Yellow` | `\x10` | Yellow |

All are `std::string_view` so they compose cleanly with `std::format`.

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

`ParseNamed` is case-insensitive and accepts `"red"`, `"green"`, `"lightblue"`, `"silver"` (alias of `lightred`), `"orange"` (alias of `gold`), etc. Unknown names return `Default`.

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
