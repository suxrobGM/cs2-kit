# Database {#database_guide}

[TOC]

## Overview

The database module (`CS2Kit::Database`) is an **opt-in** PostgreSQL layer:

- **PostgresDatabase** - one long-lived connection, a named prepared-statement cache, and lazy
  reconnect after a dropped socket
- **RunMigrations** - forward-only `NNNN_*.sql` migration runner under a session advisory lock
- **DbResult / TryDb / TryOr** - `std::expected`-based helpers that turn thrown pqxx errors into
  logged results

It compiles only when `CS2KIT_ENABLE_POSTGRES` is `ON` (default `OFF`), so plugins without a
database never pull libpqxx.

## Enabling

In a monorepo that vendors cs2-kit, declare `libpqxx` in the consuming repo's conanfile and turn
the option on before adding the kit:

```cmake
set(CS2KIT_ENABLE_POSTGRES ON)
add_subdirectory(vendor/cs2-kit)
```

For standalone kit development there is a matching conan option: `with_postgres=True`.

## Connecting

`PostgresConfig` field names are lowercase so a JSON config section maps onto them directly (the
kit header stays nlohmann-free; define the `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT`
mapper in your plugin, inside the `CS2Kit::Database` namespace so ADL finds it).

```cpp
#include <CS2Kit/Database/PostgresDatabase.hpp>

CS2Kit::Database::PostgresDatabase db;
if (!db.Initialize(config))          // logs a generic, secret-free error on failure
    return false;

// Constant SQL:
auto rows = db.Execute("SELECT * FROM admins");

// Named prepared statement, prepared once on first use:
auto row = db.ExecutePrepared("find_admin", "SELECT * FROM admins WHERE steam_id = $1", steamId);

// Multi-statement work that manages its own transactions:
db.WithConnection([](pqxx::connection& conn) { /* ... */ });
```

Queries throw on failure - wrap call sites with the `DbResult.hpp` helpers to convert exceptions
into logged results (never put secrets in the `what` label).

## Migrations

`RunMigrations(db, dir, options)` scans `dir` for files named `NNNN_*.sql` (the leading integer is
the version) and applies every file above the max version recorded in the history table, in
ascending order, each in its own transaction. A session advisory lock serializes two plugin loads
racing on the same database; a missing directory is a logged no-op.

```cpp
#include <CS2Kit/Database/Migrator.hpp>

CS2Kit::Database::RunMigrations(db, "addons/my-plugin/configs/migrations",
                                {.TableName = "schema_migrations", .AdvisoryLockKey = 727274});
```

Plugins sharing one database should use distinct table names **and** distinct lock keys. The table
name is interpolated into SQL, so it is validated against `[A-Za-z_][A-Za-z0-9_]*`.

## Threading

Main-thread only, like the rest of the kit. The single internal mutex exists so a future
off-thread query path stays safe; it is not an invitation to query from worker threads.
