#include "Sdk/Schema.hpp"

#include <CS2Kit/Sdk/EntityRender.hpp>
#include <entity2/entityinstance.h>

namespace CS2Kit::Sdk
{

void SetEntityRender(CEntityInstance* entity, RenderMode_t mode, uint32_t color)
{
    if (!entity)
        return;

    auto& schema = SchemaService::Instance();
    int modeOffset = schema.GetOffset("CBaseModelEntity", "m_nRenderMode");
    int colorOffset = schema.GetOffset("CBaseModelEntity", "m_clrRender");
    if (modeOffset < 0 || colorOffset < 0)
        return;

    auto* base = reinterpret_cast<uint8_t*>(entity);
    *reinterpret_cast<uint8_t*>(base + modeOffset) = static_cast<uint8_t>(mode);
    *reinterpret_cast<uint32_t*>(base + colorOffset) = color;
}

}  // namespace CS2Kit::Sdk
