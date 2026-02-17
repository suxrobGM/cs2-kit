#pragma once

#include <cstdint>
#include <string>

namespace CS2Kit::Utils
{

/** Static utilities for Unix timestamps, duration parsing/formatting, and expiration checks. */
class TimeUtils
{
public:
    static int64_t Now();
    static int64_t ParseDuration(const std::string& duration);
    static std::string FormatDuration(int64_t seconds);
    static std::string FormatTimestamp(int64_t timestamp);
    static bool IsExpired(int64_t expiresAt);
    static int64_t GetExpirationTime(int64_t durationSeconds);

private:
    static constexpr int64_t SecondsPerMinute = 60;
    static constexpr int64_t SecondsPerHour = 3600;
    static constexpr int64_t SecondsPerDay = 86400;
    static constexpr int64_t SecondsPerWeek = 604800;
};

}  // namespace CS2Kit::Utils
