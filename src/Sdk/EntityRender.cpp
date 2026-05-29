#include "Sdk/Schema.hpp"
#include <CS2Kit/Core/Services.hpp>

#include <CS2Kit/Sdk/EntityRender.hpp>
#include <entity2/entityinstance.h>

using CS2Kit::Core::Kit;

namespace CS2Kit::Sdk
{

void SetEntityRender(CEntityInstance* entity, RenderMode_t mode, uint32_t color)
{
    if (!entity)
        return;

    // m_nRenderMode/m_clrRender offsets are fixed by the loaded game binary, so resolve them
    // once (this runs every disco tick). Cached only on success so an early failed lookup retries.
    static int modeOffset = -1;
    static int colorOffset = -1;
    if (modeOffset < 0 || colorOffset < 0)
    {
        auto& schema = Kit().Schema();
        modeOffset = schema.GetOffset("CBaseModelEntity", "m_nRenderMode");
        colorOffset = schema.GetOffset("CBaseModelEntity", "m_clrRender");
        if (modeOffset < 0 || colorOffset < 0)
            return;
    }

    auto* base = reinterpret_cast<uint8_t*>(entity);
    *reinterpret_cast<uint8_t*>(base + modeOffset) = static_cast<uint8_t>(mode);
    *reinterpret_cast<uint32_t*>(base + colorOffset) = color;
}

}  // namespace CS2Kit::Sdk
