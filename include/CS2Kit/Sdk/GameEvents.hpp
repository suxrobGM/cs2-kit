#pragma once

#include <string>

class IGameEvent;

namespace CS2Kit::Sdk::Events
{

/**
 * @brief Typed views over the game events plugins commonly consume.
 *
 * Each struct names its engine event and decodes the raw fields once in From(), so handlers
 * read `e.VictimSlot` instead of `event->GetPlayerSlot("userid").Get()`. Subscribe with
 * @ref GameEventService::Listen<T>:
 *
 * @code
 * Engine().Events.Listen<Events::PlayerDeath>([](const auto& e) {
 *     if (e.VictimSlot >= 0) ...;
 * });
 * @endcode
 *
 * Slots decode via GetPlayerSlot (userid -> slot; -1 when absent). Events not modeled here
 * are still reachable through the string overload of Listen - add a struct when a plugin
 * starts caring about one.
 */

struct PlayerDeath
{
    static constexpr const char* Name = "player_death";
    int VictimSlot = -1;
    int AttackerSlot = -1;
    std::string Weapon;
    bool Headshot = false;
    static PlayerDeath From(IGameEvent& e);
};

struct PlayerSpawn
{
    static constexpr const char* Name = "player_spawn";
    int Slot = -1;
    static PlayerSpawn From(IGameEvent& e);
};

struct PlayerHurt
{
    static constexpr const char* Name = "player_hurt";
    int VictimSlot = -1;
    int AttackerSlot = -1;
    int Health = 0;
    int DamageHealth = 0;
    static PlayerHurt From(IGameEvent& e);
};

struct PlayerTeam
{
    static constexpr const char* Name = "player_team";
    int Slot = -1;
    int Team = 0;
    int OldTeam = 0;
    bool Disconnect = false;
    static PlayerTeam From(IGameEvent& e);
};

struct PlayerConnectFull
{
    static constexpr const char* Name = "player_connect_full";
    int Slot = -1;
    static PlayerConnectFull From(IGameEvent& e);
};

struct WeaponFire
{
    static constexpr const char* Name = "weapon_fire";
    int Slot = -1;
    std::string Weapon;
    static WeaponFire From(IGameEvent& e);
};

struct RoundStart
{
    static constexpr const char* Name = "round_start";
    static RoundStart From(IGameEvent& e);
};

struct RoundEnd
{
    static constexpr const char* Name = "round_end";
    int Winner = 0;
    int Reason = 0;
    static RoundEnd From(IGameEvent& e);
};

struct RoundPrestart
{
    static constexpr const char* Name = "round_prestart";
    static RoundPrestart From(IGameEvent& e);
};

}  // namespace CS2Kit::Sdk::Events
