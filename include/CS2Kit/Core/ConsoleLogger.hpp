#pragma once

#include <CS2Kit/Core/ILogger.hpp>

#include <string>

namespace CS2Kit::Core
{

/**
 * Default console logger implementation using HL2SDK's ConColorMsg.
 * Created automatically by CS2Kit::Initialize() when no custom logger is provided.
 */
class ConsoleLogger : public ILogger
{
public:
    void SetPrefix(const char* prefix) { _prefix = prefix; }

    void Info(const std::string& message) override;
    void Warn(const std::string& message) override;
    void Error(const std::string& message) override;

private:
    const char* _prefix = "CS2Kit";
};

}  // namespace CS2Kit::Core
