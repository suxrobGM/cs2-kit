# CS2-Kit {#mainpage}

> **Work in Progress** — This library is under active development. The API is unstable and subject to breaking changes. Features may be added, removed, or redesigned without notice.

**CS2-Kit** is a C++23 library for building [Counter-Strike 2](https://www.counter-strike.net/) server plugins with [Metamod:Source 2.0](https://www.metamodsource.net/). It provides reusable abstractions over the HL2SDK so plugin developers can focus on game logic instead of engine internals.

<h2>Modules</h2>

| Module | Namespace | Description |
|--------|-----------|-------------|
| **Core** | `CS2Kit::Core` | Singleton base, scheduler, logging and path resolution interfaces |
| **Commands** | `CS2Kit::Commands` | Chat command system with fluent builder, permissions, and dispatch |
| **Menu** | `CS2Kit::Menu` | WASD-navigated center-HTML menu framework with builder pattern |
| **Sdk** | `CS2Kit::Sdk` | Engine SDK wrappers: entities, schema, signatures, convars, events, messages |
| **Utils** | `CS2Kit::Utils` | SteamID conversions, string utilities, time formatting, translations |

<h2>Guides</h2>

- @subpage getting_started — Prerequisites and integration guide
- @subpage architecture — Design patterns and module overview
- @subpage commands_guide — Command system usage
- @subpage menus_guide — Menu system usage
- @subpage sdk_guide — SDK wrappers guide

<h2>License</h2>

CS2-Kit is released under the [MIT License](https://github.com/suxrobGM/cs2-kit/blob/main/LICENSE).
