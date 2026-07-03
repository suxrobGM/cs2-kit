#include "MicroTest.hpp"

#include <CS2Kit/Core/EffectManager.hpp>
#include <CS2Kit/Core/Scheduler.hpp>

using CS2Kit::Core::EffectManager;
using CS2Kit::Core::EffectSpec;
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
    mgr.Apply(3, Disco, {.OnStop = [] {}});
    CHECK(mgr.IsActive(3, Disco));
    CHECK(!mgr.IsActive(3, Ghost));
    CHECK(!mgr.IsActive(4, Disco));
}

TEST_CASE("EffectManager: re-Apply runs the prior onStop first")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    int cancels = 0;
    mgr.Apply(3, Disco, {.OnStop = [&] { ++cancels; }});
    mgr.Apply(3, Disco, {.OnStop = [&] { cancels += 10; }});
    CHECK_EQ(cancels, 1);  // first instance undone, second still active

    mgr.Cancel(3, Disco);
    CHECK_EQ(cancels, 11);
}

TEST_CASE("EffectManager: Cancel clears state and flips IsActive")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    int cancels = 0;
    mgr.Apply(3, Disco, {.OnStop = [&] { ++cancels; }});
    CHECK(mgr.IsActive(3, Disco));

    mgr.Cancel(3, Disco);
    CHECK(!mgr.IsActive(3, Disco));
    CHECK_EQ(cancels, 1);
}

TEST_CASE("EffectManager: Cancel is idempotent and per-id")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    int cancels = 0;
    mgr.Apply(3, Disco, {.OnStop = [&] { ++cancels; }});
    mgr.Apply(3, Ghost, {.OnStop = [&] { ++cancels; }});

    mgr.Cancel(3, Disco);
    mgr.Cancel(3, Disco);  // no-op: already gone
    CHECK_EQ(cancels, 1);
    CHECK(mgr.IsActive(3, Ghost));
}

TEST_CASE("EffectManager: CancelRoundScoped only touches round-scoped effects")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    mgr.Apply(3, Disco, {.RoundScoped = true, .OnStop = [] {}});
    mgr.Apply(3, Ghost, {.RoundScoped = false, .OnStop = [] {}});
    mgr.Apply(5, Disco, {.RoundScoped = true, .OnStop = [] {}});

    mgr.CancelRoundScoped();
    CHECK(!mgr.IsActive(3, Disco));
    CHECK(mgr.IsActive(3, Ghost));
    CHECK(!mgr.IsActive(5, Disco));
}

TEST_CASE("EffectManager: CancelAllForSlot and CancelAll")
{
    Scheduler scheduler;
    EffectManager mgr(scheduler);

    mgr.Apply(3, Disco, {.OnStop = [] {}});
    mgr.Apply(3, Ghost, {.OnStop = [] {}});
    mgr.Apply(5, Disco, {.OnStop = [] {}});

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

    mgr.Apply(-1, Disco, {.OnStop = [] {}});
    mgr.Apply(EffectManager::MaxSlots, Disco, {.OnStop = [] {}});
    CHECK(!mgr.IsActive(-1, Disco));
    CHECK(!mgr.IsActive(EffectManager::MaxSlots, Disco));
    mgr.Cancel(-1, Disco);
    mgr.CancelAllForSlot(EffectManager::MaxSlots);  // must not crash
}
