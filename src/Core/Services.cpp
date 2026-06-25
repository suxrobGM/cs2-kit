#include "Sdk/Schema.hpp"

#include <CS2Kit/Core/ActiveService.hpp>
#include <CS2Kit/Core/Services.hpp>

namespace CS2Kit::Core
{

Services::Services() : _schema(std::make_unique<Sdk::SchemaService>()) {}

Services::~Services() = default;

void SetActiveServices(Services* services)
{
    ActiveService<Services>::Set(services);
}

Services& Engine()
{
    return ActiveService<Services>::Get();
}

Services* EngineOrNull()
{
    return ActiveService<Services>::GetOrNull();
}

}  // namespace CS2Kit::Core
