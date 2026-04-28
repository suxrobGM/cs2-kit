#pragma once

#include <string>
#include <string_view>

namespace CS2Kit::Utils::ChatColors
{

/**
 * CS2 SayText2 in-line color escape codes. Embed inside chat messages to colorize
 * the text that follows. A new escape (or `Default`) ends the previous run.
 */
inline constexpr std::string_view Default = "\x01";      /**< Default white/yellow chat color. */
inline constexpr std::string_view Red = "\x02";          /**< Red. */
inline constexpr std::string_view LightPurple = "\x03";  /**< Light purple. */
inline constexpr std::string_view Green = "\x04";        /**< Green. */
inline constexpr std::string_view Olive = "\x05";        /**< Olive / dark green. */
inline constexpr std::string_view Lime = "\x06";         /**< Lime / light green. */
inline constexpr std::string_view LightRed = "\x07";     /**< Light red / silver. */
inline constexpr std::string_view Gray = "\x08";         /**< Gray. */
inline constexpr std::string_view LightYellow = "\x09";  /**< Light yellow. */
inline constexpr std::string_view LightBlue = "\x0A";    /**< Light blue. */
inline constexpr std::string_view Blue = "\x0B";         /**< Blue. */
inline constexpr std::string_view Purple = "\x0C";       /**< Purple. */
inline constexpr std::string_view Pink = "\x0D";         /**< Pink. */
inline constexpr std::string_view Gold = "\x0E";         /**< Gold / orange. */
inline constexpr std::string_view Yellow = "\x10";       /**< Yellow. */

/**
 * Map a human color name (case-insensitive) to its escape sequence.
 * Returns `Default` for unknown names. Empty input also returns `Default`.
 */
std::string_view ParseNamed(std::string_view name);

/** Strip all CS2 color escape bytes (0x01..0x10) from `text` for safe console/log output. */
std::string Strip(std::string_view text);

}  // namespace CS2Kit::Utils::ChatColors
