#pragma once

#include <cmath>

namespace CS2Kit::Utils::AngleMath
{

inline constexpr float RadToDeg = 57.29577951308232f;

/** Aim direction in Source convention: pitch positive looking down, both in degrees. */
struct AimAngles
{
    float Pitch = 0.0f;
    float Yaw = 0.0f;
};

/** Wrap an angle difference into (-180, 180]. NaN passes through. */
[[nodiscard]] inline float NormalizeAngleDelta(float degrees)
{
    degrees = std::fmod(degrees, 360.0f);
    if (degrees > 180.0f)
        degrees -= 360.0f;
    else if (degrees <= -180.0f)
        degrees += 360.0f;
    return degrees;
}

/**
 * View angles that point from @p from at @p to. Any vector type with x/y/z
 * members works (SDK Vector included); the header stays SDK-free for tests.
 */
template <class Vec>
[[nodiscard]] AimAngles AnglesToPoint(const Vec& from, const Vec& to)
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float dz = to.z - from.z;
    float horizontal = std::sqrt(dx * dx + dy * dy);
    return {
        .Pitch = -std::atan2(dz, horizontal) * RadToDeg,
        .Yaw = std::atan2(dy, dx) * RadToDeg,
    };
}

/** Combined angular error (degrees) between two view directions. */
[[nodiscard]] inline float AngularDistance(const AimAngles& a, const AimAngles& b)
{
    float dp = NormalizeAngleDelta(a.Pitch - b.Pitch);
    float dy = NormalizeAngleDelta(a.Yaw - b.Yaw);
    return std::sqrt(dp * dp + dy * dy);
}

}  // namespace CS2Kit::Utils::AngleMath
