#include "MicroTest.hpp"

#include <CS2Kit/Utils/SlotThrottle.hpp>

using CS2Kit::Utils::SlotThrottle;

TEST_CASE("SlotThrottle: first acquire always succeeds")
{
    SlotThrottle throttle(60);
    CHECK(throttle.TryAcquire(3, 1000));
}

TEST_CASE("SlotThrottle: re-acquire blocked inside interval, allowed after")
{
    SlotThrottle throttle(60);
    CHECK(throttle.TryAcquire(3, 1000));
    CHECK(!throttle.TryAcquire(3, 1030));
    CHECK(!throttle.TryAcquire(3, 1059));
    CHECK(throttle.TryAcquire(3, 1060));
}

TEST_CASE("SlotThrottle: slots are independent")
{
    SlotThrottle throttle(60);
    CHECK(throttle.TryAcquire(1, 1000));
    CHECK(throttle.TryAcquire(2, 1000));
    CHECK(!throttle.TryAcquire(1, 1010));
}

TEST_CASE("SlotThrottle: Reset re-arms a slot")
{
    SlotThrottle throttle(60);
    CHECK(throttle.TryAcquire(5, 1000));
    CHECK(!throttle.TryAcquire(5, 1010));
    throttle.Reset(5);
    CHECK(throttle.TryAcquire(5, 1011));
}
