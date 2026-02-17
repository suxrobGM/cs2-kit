#include <CS2Kit/Core/Paths.hpp>

namespace CS2Kit::Core
{

static std::filesystem::path g_baseDir;

void SetBaseDir(const std::filesystem::path& baseDir)
{
    g_baseDir = baseDir;
}

std::filesystem::path ResolvePath(const std::string& relativePath)
{
    std::filesystem::path p(relativePath);
    return p.is_absolute() ? p : g_baseDir / p;
}

}  // namespace CS2Kit::Core
