#include <CS2Kit/Database/Migrator.hpp>
#include <CS2Kit/Database/PostgresDatabase.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace CS2Kit::Database
{

namespace Log = CS2Kit::Utils::Log;

namespace
{

struct Migration
{
    int Version;
    std::string Name;
    fs::path Path;
};

/** `[A-Za-z_][A-Za-z0-9_]*` - the table name is interpolated into SQL, so reject anything else. */
bool IsValidTableName(const std::string& name)
{
    if (name.empty())
        return false;
    if (!std::isalpha(static_cast<unsigned char>(name.front())) && name.front() != '_')
        return false;
    return std::all_of(name.begin(), name.end(), [](unsigned char c) { return std::isalnum(c) || c == '_'; });
}

std::string ReadFile(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace

bool RunMigrations(PostgresDatabase& db, const std::string& dir, const MigrationOptions& options)
{
    if (!IsValidTableName(options.TableName))
    {
        Log::Error("Invalid migration table name '{}'; refusing to run migrations.", options.TableName);
        return false;
    }
    const std::string& table = options.TableName;

    std::error_code ec;
    if (!fs::exists(dir, ec))
    {
        Log::Warn("Migrations directory not found ({}); skipping schema setup.", dir);
        return true;
    }

    std::vector<Migration> migrations;
    for (const auto& entry : fs::directory_iterator(dir, ec))
    {
        if (!entry.is_regular_file())
            continue;
        std::string name = entry.path().filename().string();
        if (!name.ends_with(".sql"))
            continue;
        auto version = ParseMigrationVersion(name);
        if (!version)  // ignore stray files without a leading version (e.g. *.sql.bak)
            continue;
        migrations.push_back({*version, name, entry.path()});
    }
    std::sort(migrations.begin(), migrations.end(),
              [](const Migration& a, const Migration& b) { return a.Version < b.Version; });

    try
    {
        return db.WithConnection([&](pqxx::connection& conn) -> bool {
            {
                pqxx::work txn(conn);
                txn.exec("CREATE TABLE IF NOT EXISTS " + table +
                         " (version INTEGER PRIMARY KEY, "
                         "name TEXT NOT NULL, "
                         "applied_at BIGINT NOT NULL DEFAULT EXTRACT(EPOCH FROM NOW())::BIGINT)");
                txn.commit();
            }

            // Session-level advisory lock held across the per-file transactions below; auto-released if
            // the connection drops. Serializes two plugin loads that race on the same database.
            {
                pqxx::work txn(conn);
                txn.exec("SELECT pg_advisory_lock(" + std::to_string(options.AdvisoryLockKey) + ")");
                txn.commit();
            }

            int current = 0;
            {
                pqxx::work txn(conn);
                pqxx::result r = txn.exec("SELECT COALESCE(MAX(version), 0) FROM " + table);
                current = r[0][0].as<int>();
                txn.commit();
            }

            int applied = 0;
            bool ok = true;
            for (const Migration& m : migrations)
            {
                if (m.Version <= current)
                    continue;
                try
                {
                    pqxx::work txn(conn);
                    txn.exec(ReadFile(m.Path));
                    txn.exec("INSERT INTO " + table + " (version, name) VALUES ($1, $2)",
                             pqxx::params{m.Version, m.Name});
                    txn.commit();
                    ++applied;
                    Log::Info("Applied migration {} ({}).", m.Version, m.Name);
                }
                catch (const std::exception& e)
                {
                    Log::Error("Migration {} ({}) failed: {}", m.Version, m.Name, e.what());
                    ok = false;
                    break;
                }
            }

            {
                pqxx::work txn(conn);
                txn.exec("SELECT pg_advisory_unlock(" + std::to_string(options.AdvisoryLockKey) + ")");
                txn.commit();
            }

            if (applied > 0)
                Log::Info("Database schema up to date ({} migration(s) applied).", applied);
            return ok;
        });
    }
    catch (const std::exception& e)
    {
        Log::Error("Migration runner failed: {}", e.what());
        return false;
    }
}

}  // namespace CS2Kit::Database
