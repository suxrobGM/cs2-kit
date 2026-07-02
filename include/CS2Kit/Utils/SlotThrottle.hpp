#pragma once

#include <cstdint>
#include <unordered_map>

namespace CS2Kit::Utils
{

/**
 * @brief Per-slot rate limiter for notices triggered by spammy hooks (per-keypress voice
 * callbacks, per-message chat handlers). @ref TryAcquire returns true at most once per
 * interval for a given slot; the first call for a slot always succeeds.
 */
class SlotThrottle
{
public:
    explicit SlotThrottle(int64_t intervalSec) : _intervalSec(intervalSec) {}

    /** True (recording @p nowSec) when at least the interval has passed since the last acquire. */
    bool TryAcquire(int slot, int64_t nowSec)
    {
        auto& last = _lastAt[slot];
        if (nowSec - last < _intervalSec)
            return false;
        last = nowSec;
        return true;
    }

    /** Forget a slot's history (e.g. on disconnect) so the slot's next occupant is notified immediately. */
    void Reset(int slot) { _lastAt.erase(slot); }

private:
    int64_t _intervalSec;
    std::unordered_map<int, int64_t> _lastAt;
};

}  // namespace CS2Kit::Utils
