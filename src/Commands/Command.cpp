#include <CS2Kit/Commands/Command.hpp>

#include <CS2Kit/Utils/StringUtils.hpp>

namespace CS2Kit::Commands
{

using namespace CS2Kit::Utils;

bool Command::Matches(const std::string& cmd) const
{
    auto lower = StringUtils::ToLower(cmd);
    if (StringUtils::ToLower(Name) == lower)
        return true;
    for (const auto& alias : Aliases)
    {
        if (StringUtils::ToLower(alias) == lower)
            return true;
    }
    return false;
}

}  // namespace CS2Kit::Commands
