#pragma once

#include <cstdint>
#include <string>

namespace CS2Kit::Commands
{

/**
 * @brief Interface representation of a command caller (player or server console).
 * Consumers implement this to wrap their Player class or console representation.
 */
class ICommandCaller
{
public:
    virtual ~ICommandCaller() = default;

    virtual int64_t GetSteamID() const = 0;
    virtual std::string GetName() const = 0;
    virtual bool IsServerConsole() const = 0;
};

}  // namespace CS2Kit::Commands
