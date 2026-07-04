#pragma once

#include <CS2Kit/Sdk/MoveType.hpp>
#include <CS2Kit/Sdk/PlayerController.hpp>
#include <cstdint>

namespace CS2Kit::Sdk
{

/**
 * @brief Pawn-state predicate factories for state-toggle menu rows (re-read every redraw).
 */

/** The pawn is currently in @p activeType (e.g. MoveType::None = frozen). */
inline auto InMoveType(MoveType activeType)
{
    return [activeType](const PlayerController& pc) { return pc.GetMoveType() == activeType; };
}

/** An m_fFlags bit is set on the pawn (e.g. FL_GODMODE). */
inline auto HasPawnFlag(uint32_t flag)
{
    return [flag](const PlayerController& pc) { return (pc.GetFlags() & flag) != 0; };
}

}  // namespace CS2Kit::Sdk
