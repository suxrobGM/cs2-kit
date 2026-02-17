#pragma once

#include <filesystem>
#include <string>

namespace CS2Kit::Core
{

/**
 * @brief Set the base directory for path resolution (typically from ISmmAPI::GetBaseDir()).
 * Must be called during initialization before any file loading.
 *
 * @param baseDir The base directory path to use for resolving relative paths.
 */
void SetBaseDir(const std::filesystem::path& baseDir);

/**
 * @brief  a relative path against the base directory.
 * If the path is already absolute, returns it as-is.
 * @param relativePath The relative path to resolve.
 * @return The resolved absolute path.
 */
std::filesystem::path ResolvePath(const std::string& relativePath);

}  // namespace CS2Kit::Core
