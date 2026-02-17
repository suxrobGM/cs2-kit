#include "ILogger.hpp"

namespace CS2Kit::Core
{

static ILogger* g_logger = nullptr;

ILogger* GetGlobalLogger() { return g_logger; }
void SetGlobalLogger(ILogger* logger) { g_logger = logger; }

}  // namespace CS2Kit::Core
