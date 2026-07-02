#include <CS2Kit/Database/PostgresDatabase.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <format>
#include <stdexcept>

namespace CS2Kit::Database
{

std::string PostgresConfig::GetConnectionString() const
{
    return std::format("host={} port={} dbname={} user={} password={} sslmode={}", host, port, database, username,
                       password, sslMode);
}

bool PostgresDatabase::Initialize(const PostgresConfig& config)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _connectionString = config.GetConnectionString();

    try
    {
        EnsureOpen();
        return true;
    }
    catch (const std::exception&)
    {
        // Don't log the exception text: a failed pqxx::connection ctor can echo the full DSN
        // (password included). Report a generic, secret-free message instead.
        CS2Kit::Utils::Log::Error("Database connection failed - check host/port/credentials.");
        return false;
    }
}

void PostgresDatabase::CloseConnection()
{
    std::lock_guard<std::mutex> lock(_mutex);
    Reset();
}

bool PostgresDatabase::IsConnected() const
{
    return _connection && _connection->is_open();
}

pqxx::result PostgresDatabase::Execute(const std::string& query)
{
    std::lock_guard<std::mutex> lock(_mutex);
    EnsureOpen();

    pqxx::work txn(*_connection);
    pqxx::result result = txn.exec(query);
    txn.commit();
    return result;
}

void PostgresDatabase::EnsureOpen()
{
    if (_connection && _connection->is_open())
        return;

    // A reopened socket has no server-side prepared statements; forget the cache so each name is
    // re-prepared on first use against the new connection.
    _prepared.clear();
    _connection = std::make_unique<pqxx::connection>(_connectionString);
    if (!_connection->is_open())
    {
        _connection.reset();
        throw std::runtime_error("Failed to open database connection");
    }
}

void PostgresDatabase::Reset()
{
    _connection.reset();
    _prepared.clear();
}

}  // namespace CS2Kit::Database
