#include <CS2Kit/Core/ConsoleLogger.hpp>

#include <Color.h>
#include <format>
#include <tier0/dbg.h>

namespace CS2Kit::Core
{

void ConsoleLogger::Info(const std::string& message)
{
    ConColorMsg(Color(0, 255, 0, 255), "%s\n", std::format("[{}] {}", _prefix, message).c_str());
}

void ConsoleLogger::Warn(const std::string& message)
{
    ConColorMsg(Color(255, 255, 0, 255), "%s\n", std::format("[{}] WARN: {}", _prefix, message).c_str());
}

void ConsoleLogger::Error(const std::string& message)
{
    ConColorMsg(Color(255, 0, 0, 255), "%s\n", std::format("[{}] ERROR: {}", _prefix, message).c_str());
}

}  // namespace CS2Kit::Core
