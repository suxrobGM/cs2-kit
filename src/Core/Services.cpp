#include "Sdk/Schema.hpp"

#include <CS2Kit/Core/Services.hpp>

#include <cassert>

namespace CS2Kit::Core
{

Services::Services() : _schema(std::make_unique<Sdk::SchemaService>()) {}

Services::~Services() = default;

namespace
{
Services* g_active = nullptr;
}  // namespace

void SetActiveServices(Services* services)
{
    g_active = services;
}

Services& Kit()
{
    assert(g_active && "CS2Kit::Kit() called with no active Services (outside Load/Unload)");
    return *g_active;
}

Services* KitOrNull()
{
    return g_active;
}

}  // namespace CS2Kit::Core
