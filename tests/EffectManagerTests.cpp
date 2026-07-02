#include "MicroTest.hpp"

#include <CS2Kit/Core/EffectManager.hpp>
#include <CS2Kit/Core/Scheduler.hpp>

using CS2Kit::Core::EffectManager;
using CS2Kit::Core::EffectSetup;
using CS2Kit::Core::Scheduler;

namespace
{
constexpr int Disco = 0;
constexpr int Ghost = 1;
}  // namespace

TEST_CASE("EffectManager: Apply and IsActive")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    CHECK(!mgr.IsActive(3, Disco));
    mgr.Apply(3, Disco, 0, [] {});
    CHECK(mgr.IsActive(3, Disco));
    CHECK(!mgr.IsActive(3, Ghost));
    CHECK(!mgr.IsActive(4, Disco));
}

TEST_CASE("EffectManager: re-Apply runs the prior cancel first")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    int cancels = 0;
    mgr.Apply(3, Disco, 0, [&] { ++cancels; });
    mgr.Apply(3, Disco, 0, [&] { cancels += 10; });
    CHECK_EQ(cancels, 1);  // first instance undone, second still active

    mgr.Cancel(3, Disco);
    CHECK_EQ(cancels, 11);
}

TEST_CASE("EffectManager: Toggle returns on-state and cancels on re-toggle")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    int cancels = 0;
    auto enable = [&]() -> EffectSetup { return {.CancelFn = [&] { ++cancels; }}; };

    CHECK(mgr.Toggle(3, Disco, enable));
    CHECK(mgr.IsActive(3, Disco));
    CHECK(!mgr.Toggle(3, Disco, enable));
    CHECK(!mgr.IsActive(3, Disco));
    CHECK_EQ(cancels, 1);
}

TEST_CASE("EffectManager: Cancel is idempotent and per-id")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    int cancels = 0;
    mgr.Apply(3, Disco, 0, [&] { ++cancels; });
    mgr.Apply(3, Ghost, 0, [&] { ++cancels; });

    mgr.Cancel(3, Disco);
    mgr.Cancel(3, Disco);  // no-op: already gone
    CHECK_EQ(cancels, 1);
    CHECK(mgr.IsActive(3, Ghost));
}

TEST_CASE("EffectManager: CancelRoundScoped only touches round-scoped effects")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    mgr.Apply(3, Disco, 0, [] {}, /*roundScoped*/ true);
    mgr.Apply(3, Ghost, 0, [] {}, /*roundScoped*/ false);
    mgr.Apply(5, Disco, 0, [] {}, /*roundScoped*/ true);

    mgr.CancelRoundScoped();
    CHECK(!mgr.IsActive(3, Disco));
    CHECK(mgr.IsActive(3, Ghost));
    CHECK(!mgr.IsActive(5, Disco));
}

TEST_CASE("EffectManager: CancelAllForSlot and CancelAll")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    mgr.Apply(3, Disco, 0, [] {});
    mgr.Apply(3, Ghost, 0, [] {});
    mgr.Apply(5, Disco, 0, [] {});

    mgr.CancelAllForSlot(3);
    CHECK(!mgr.IsActive(3, Disco));
    CHECK(!mgr.IsActive(3, Ghost));
    CHECK(mgr.IsActive(5, Disco));

    mgr.CancelAll();
    CHECK(!mgr.IsActive(5, Disco));
}

TEST_CASE("EffectManager: out-of-range slots are no-ops")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    mgr.Apply(-1, Disco, 0, [] {});
    mgr.Apply(EffectManager::MaxSlots, Disco, 0, [] {});
    CHECK(!mgr.IsActive(-1, Disco));
    CHECK(!mgr.IsActive(EffectManager::MaxSlots, Disco));
    mgr.Cancel(-1, Disco);
    mgr.CancelAllForSlot(EffectManager::MaxSlots);  // must not crash
}
