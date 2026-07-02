# CS2-Kit {#mainpage}

> **Work in Progress** - This library is under active development. The API is unstable and subject to breaking changes. Features may be added, removed, or redesigned without notice.

**CS2-Kit** is a C++23 library for building [Counter-Strike 2](https://www.counter-strike.net/) server plugins with [Metamod:Source 2.0](https://www.metamodsource.net/). It provides reusable abstractions over the HL2SDK so plugin developers can focus on game logic instead of engine internals.

<h2>Modules</h2>

| Module | Namespace | Description |
|--------|-----------|-------------|
| **Core** | `CS2Kit::Core` | Plugin base (`MetamodPluginBase`), service container, scheduler, per-slot effect registry (`EffectManager`), logging (built-in ConsoleLogger), path resolution |
| **Commands** | `CS2Kit::Commands` | Chat command system with fluent builder, permissions, and dispatch |
| **Database** | `CS2Kit::Database` | Optional PostgreSQL client (`PostgresDatabase`), forward-only migration runner, `DbResult` error helpers; gated behind `CS2KIT_ENABLE_POSTGRES` |
| **Http** | `CS2Kit::Http` | Async `HttpClient` (game-thread completions via the frame pump) and config-driven JSON POST helpers (`RestJsonApi`) |
| **Menu** | `CS2Kit::Menu` | WASD-navigated center-HTML menu framework with builder pattern and presets (player/duration pickers, confirm dialog, palette choices) |
| **Players** | `CS2Kit::Players` | Connected-player tracking, target-token resolution (`ResolveTargets`), and the policy-injected `ActionDispatcher` |
| **Sdk** | `CS2Kit::Sdk` | Engine SDK wrappers: entities, schema, signatures, convars, events, messages, pawn operations (`PawnOps`), persistent center-HTML panels |
| **Utils** | `CS2Kit::Utils` | JSON serializer, SteamID conversions, string utilities (HTML escaping, UTF-8-safe truncation), time/duration formatting, token-substituting translations, colored chat output, per-slot throttling |

<h2>Guides</h2>

- @subpage getting_started - Prerequisites and integration guide
- @subpage architecture - Design patterns and module overview
- @subpage plugin_guide - Building a plugin with MetamodPluginBase
- @subpage commands_guide - Command system usage
- @subpage menus_guide - Menu system usage
- @subpage players_guide - Player tracking, target resolution, and actions
- @subpage chat_guide - Colored chat output
- @subpage sdk_guide - SDK wrappers guide
- @subpage database_guide - PostgreSQL client and migrations
- @subpage http_guide - Async HTTP and JSON REST helpers

<h2>License</h2>

CS2-Kit is released under the [MIT License](https://github.com/suxrobGM/cs2-kit/blob/main/LICENSE).
