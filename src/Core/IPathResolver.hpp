#pragma once

#include <filesystem>
#include <string>

namespace CS2Kit::Core
{

/**
 * Path resolution interface for resolving relative paths to absolute paths.
 *
 * Consumers implement this to provide their base directory (e.g., CS2 server root).
 * The library uses this for loading configuration files, gamedata, translations, etc.
 */
class IPathResolver
{
public:
    virtual ~IPathResolver() = default;

    /**
     * Resolve a relative path to an absolute path.
     * @param relativePath Path to resolve (e.g., "addons/myplugin/configs/settings.json").
     * @return Absolute path. If relativePath is already absolute, return as-is.
     */
    virtual std::filesystem::path ResolvePath(const std::string& relativePath) const = 0;
};

/** Global path resolver accessor. Consumers must call SetGlobalPathResolver() during initialization. */
IPathResolver* GetGlobalPathResolver();
void SetGlobalPathResolver(IPathResolver* resolver);

}  // namespace CS2Kit::Core
