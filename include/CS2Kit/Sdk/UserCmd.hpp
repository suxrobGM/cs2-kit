#pragma once

#include <cstdint>

namespace CS2Kit::Sdk
{

/**
 * @brief Mutation helpers for the CUserCmd a MovementHook pre-callback receives.
 *
 * CS2 input is subtick: each user command carries timestamped button events ("jump pressed
 * 37% into this tick"), and the movement code acts on those events - not on the held-button
 * mask. Injecting an event before the command is processed makes the engine behave exactly
 * as if the player had produced it, going through its own jump/attack/etc. paths.
 *
 * The pointer is the engine's own deserialized command object; these helpers reinterpret it
 * with the kit's compiled protobuf classes (CSGOUserCmdPB at +0x10, layout verified against
 * CS2Fixes and SwiftlyS2), so they must be kept in sync with protobufs/cs_usercmd.proto.
 */

/** Append a subtick button event to @p userCmd. @p when is the fraction of the tick [0,1].
 *  For a button the player is already holding, inject a release edge first - the engine's
 *  input state machine only reacts to transitions. Returns false on null @p userCmd. */
bool InjectSubtickPress(void* userCmd, uint64_t button, bool pressed, float when);

/** True when @p userCmd already carries a pressed=true event for @p button. */
bool HasSubtickPress(void* userCmd, uint64_t button);

}  // namespace CS2Kit::Sdk
