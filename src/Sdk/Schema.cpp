#include "Schema.hpp"

#include "GameInterfaces.hpp"
#include "../Utils/Log.hpp"

#include <schemasystem/schemasystem.h>

using namespace CS2Kit::Utils;

namespace CS2Kit::Sdk
{

bool SchemaService::Initialize()
{
    if (!GameInterfaces::Instance().SchemaSystem)
    {
        Log::Warn("ISchemaSystem not available.");
        return false;
    }

    Log::Info("Schema system initialized.");
    return true;
}

int SchemaService::GetOffset(const char* className, const char* fieldName)
{
    auto* schemaSystem = GameInterfaces::Instance().SchemaSystem;
    if (!schemaSystem)
        return -1;

    auto classIt = _offsetCache.find(className);
    if (classIt != _offsetCache.end())
    {
        auto fieldIt = classIt->second.find(fieldName);
        if (fieldIt != classIt->second.end())
            return fieldIt->second;
    }

#ifdef _WIN32
    const char* moduleName = "server.dll";
#else
    const char* moduleName = "libserver.so";
#endif

    CSchemaSystemTypeScope* pTypeScope = schemaSystem->FindTypeScopeForModule(moduleName);
    if (!pTypeScope)
    {
        Log::Error("Schema: Failed to find type scope for {}.", moduleName);
        return -1;
    }

    SchemaMetaInfoHandle_t<CSchemaClassInfo> hClassInfo = pTypeScope->FindDeclaredClass(className);
    CSchemaClassInfo* pClassInfo = hClassInfo.Get();
    if (!pClassInfo)
    {
        Log::Error("Schema: Class '{}' not found.", className);
        return -1;
    }

    for (int i = 0; i < pClassInfo->m_nFieldCount; ++i)
    {
        SchemaClassFieldData_t& field = pClassInfo->m_pFields[i];
        if (strcmp(field.m_pszName, fieldName) == 0)
        {
            int offset = field.m_nSingleInheritanceOffset;
            _offsetCache[className][fieldName] = offset;
            Log::Info("Schema: {}::{} = 0x{:X} ({})", className, fieldName, offset, offset);
            return offset;
        }
    }

    Log::Warn("Schema: Field '{}' not found in '{}'.", fieldName, className);
    return -1;
}

}  // namespace CS2Kit::Sdk
