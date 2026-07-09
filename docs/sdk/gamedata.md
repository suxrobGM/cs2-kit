# Gamedata & Schema {#sdk_gamedata_guide}

[TOC]

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

### Eager resolution and diagnostics

`CS2Kit::Initialize()` runs `ResolveAll()` as the `GameData` load stage: every signature is scanned once, `FindSignature`/`ResolveSignature` answer from the cache afterwards, and the stage reports failures by name (`"2/13 signatures failed: X, Y"`). The scanner also detects **ambiguous** patterns - a pattern matching more than one location is a broken signature waiting to resolve to the wrong function after a game update, so it is warned about and listed in the stage detail. Per-entry results are available programmatically via `Resolutions()`.

### Deliberately not implemented

Two s2sdk-style mechanisms were evaluated and rejected for now; revisit if an engine update actually burns us:

- **Ref-anchored fallback** (find functions by referenced strings when a pattern breaks): high machinery cost for a gamedata file this small whose signatures are synced from upstream projects with provenance comments.
- **RTTI vtable-by-name resolution**: current consumers are index-based vcalls whose indexes historically break less often than byte patterns.

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

## Signature scanning

The low-level byte-pattern scanner is **internal** (`src/Sdk/SigScanner.hpp`, free functions
`FindPattern(moduleName, pattern)` / `ResolveRelativeAddress(addr, ripOffset, ripSize)`); it is not
part of the public include tree. Consumers scan through @ref CS2Kit::Sdk::GameData instead, which
adds per-platform patterns, named lookups, and caching:

```cpp
auto& gd = Engine().GameData;
void* addr = gd.FindSignature("CCSPlayerController_Kick");     // raw match
void* resolved = gd.ResolveSignature("SomeFunction");          // + RIP-relative resolve
```

Wildcard bytes are written as `?` or `??` in pattern strings (see the signatures.jsonc format above).

## SchemaService

Resolves entity field offsets at runtime using CS2's schema system. Results are cached:

```cpp
auto& schema = Engine().Schema();

// Get field offset
int32_t offset = schema.GetOffset("CCSPlayerPawn", "m_iHealth");

// Pass the expected size for fixed-size reads/writes: the first lookup validates it
// against the engine's field size and warns "is N bytes but the caller expects M
// (schema drift?)" after a game update changes a field type; still returns the offset.
int32_t checked = schema.GetOffset("CCSPlayerPawn", "m_iHealth", sizeof(int));

// Use with entity pointer
int health = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pawn) + offset);
```

`PlayerController`'s typed field templates (`GetField<T>`, `GetPawnField<T>`, `SetField<T>`, `SetPawnField<T>`) pass `sizeof(T)` through automatically.
