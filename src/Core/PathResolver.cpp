#include "IPathResolver.hpp"

namespace CS2Kit::Core
{

static IPathResolver* g_pathResolver = nullptr;

IPathResolver* GetGlobalPathResolver() { return g_pathResolver; }
void SetGlobalPathResolver(IPathResolver* resolver) { g_pathResolver = resolver; }

}  // namespace CS2Kit::Core
