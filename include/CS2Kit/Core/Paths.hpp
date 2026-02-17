#pragma once

#include <filesystem>
#include <string>

namespace CS2Kit::Core
{

/**
 * Set the base directory for path resolution (typically from ISmmAPI::GetBaseDir()).
 * Must be called during initialization before any file loading.
 */
void SetBaseDir(const std::filesystem::path& baseDir);

/**
 * Resolve a relative path against the base directory.
 * If the path is already absolute, returns it as-is.
 */
std::filesystem::path ResolvePath(const std::string& relativePath);

}  // namespace CS2Kit::Core
