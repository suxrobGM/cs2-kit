#pragma once

#include <CS2Kit/Core/Singleton.hpp>

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
class SchemaService : public Core::Singleton<SchemaService>
{
public:
    explicit SchemaService(Token) {}

    bool Initialize();
    int GetOffset(const char* className, const char* fieldName);

private:
    std::map<std::string, std::map<std::string, int>> _offsetCache;
};

}  // namespace CS2Kit::Sdk
