#include <CS2Kit/Players/Player.hpp>
#include <CS2Kit/Utils/TimeUtils.hpp>

namespace CS2Kit::Players
{

using namespace CS2Kit::Utils;

Player::Player(int slot, int64_t steamId, const std::string& name, const std::string& ipAddress)
    : _slot(slot), _steamId(steamId), _name(name), _ipAddress(ipAddress), _connectTime(TimeUtils::Now())
{}

int64_t Player::GetPlaytime() const
{
    return TimeUtils::Now() - _connectTime;
}

}  // namespace CS2Kit::Players
