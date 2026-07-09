#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace CS2Kit::Sdk
{

/**
 * Runtime schema field offset resolution via ISchemaSystem.
 * Provides access to entity field offsets at runtime by querying the
 * engine's schema system. Results are cached for O(1) repeated access.
 */
class SchemaService
{
public:
    SchemaService() = default;

    bool Initialize();

    /**
     * Field offset via the engine's schema system (cached). When `expectedSize` > 0, the
     * first (uncached) lookup also compares the engine's field size against it and warns
     * on mismatch - catches schema drift after a game update. Warning-only.
     */
    int GetOffset(const char* className, const char* fieldName, int expectedSize = 0);

private:
    std::map<std::string, std::map<std::string, int>> _offsetCache;
};

}  // namespace CS2Kit::Sdk
