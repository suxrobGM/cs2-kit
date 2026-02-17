#pragma once

#include <string>

namespace CS2Kit::Core
{

/**
 * Logger interface that consumers implement to provide their own logging backend.
 *
 * The library uses this interface for all log output instead of directly calling
 * HL2SDK's ConColorMsg. Consumers implement this to log to console, file, or
 * any other destination.
 */
class ILogger
{
public:
    virtual ~ILogger() = default;

    virtual void Info(const std::string& message) = 0;
    virtual void Warn(const std::string& message) = 0;
    virtual void Error(const std::string& message) = 0;
};

/** Global logger accessor. Consumers must call SetGlobalLogger() during initialization. */
ILogger* GetGlobalLogger();
void SetGlobalLogger(ILogger* logger);

}  // namespace CS2Kit::Core
