#include <CS2Kit/Core/StatusService.hpp>
#include <format>

namespace CS2Kit::Core
{

void StatusService::RegisterSection(std::string name, Provider provider)
{
    for (auto& [existing, existingProvider] : _sections)
    {
        if (existing == name)
        {
            existingProvider = std::move(provider);
            return;
        }
    }
    _sections.emplace_back(std::move(name), std::move(provider));
}

nlohmann::json StatusService::BuildJson() const
{
    auto out = nlohmann::json::object();
    for (const auto& [name, provider] : _sections)
        out[name] = provider();
    return out;
}

std::string StatusService::BuildText() const
{
    std::string out;
    for (const auto& [name, provider] : _sections)
    {
        out += name;
        out += ":\n";
        const auto section = provider();
        if (section.is_object())
        {
            for (const auto& [key, value] : section.items())
                out += std::format("  {}: {}\n", key, value.is_string() ? value.get<std::string>() : value.dump());
        }
        else
        {
            out += std::format("  {}\n", section.dump());
        }
    }
    if (!out.empty() && out.back() == '\n')
        out.pop_back();
    return out;
}

}  // namespace CS2Kit::Core
