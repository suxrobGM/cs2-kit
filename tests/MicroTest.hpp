#pragma once

// Minimal self-contained test harness (no external deps).
// Each test .cpp registers cases with TEST_CASE; run-tests.sh links them with a
// single MicroTestMain.cpp that calls RunAllTests(). CHECK/CHECK_EQ count
// failures; the process exit code is nonzero if any check failed.

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace MicroTest
{

struct TestCase
{
    std::string Name;
    std::function<void()> Fn;
};

inline std::vector<TestCase>& Registry()
{
    static std::vector<TestCase> cases;
    return cases;
}

inline int& FailureCount()
{
    static int count = 0;
    return count;
}

inline int& CheckCount()
{
    static int count = 0;
    return count;
}

struct Registrar
{
    Registrar(const char* name, std::function<void()> fn) { Registry().push_back({name, std::move(fn)}); }
};

inline void ReportFailure(const char* file, int line, const std::string& expr)
{
    ++FailureCount();
    std::printf("  FAIL %s:%d: %s\n", file, line, expr.c_str());
}

inline int RunAllTests()
{
    for (const auto& tc : Registry())
    {
        int before = FailureCount();
        tc.Fn();
        bool ok = FailureCount() == before;
        std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", tc.Name.c_str());
    }
    std::printf("\n%d checks, %d failures\n", CheckCount(), FailureCount());
    return FailureCount() == 0 ? 0 : 1;
}

}  // namespace MicroTest

#define MT_CONCAT_INNER(a, b) a##b
#define MT_CONCAT(a, b)       MT_CONCAT_INNER(a, b)

#define TEST_CASE(name)                                                                               \
    static void MT_CONCAT(mt_test_, __LINE__)();                                                      \
    static ::MicroTest::Registrar MT_CONCAT(mt_reg_, __LINE__)(name, &MT_CONCAT(mt_test_, __LINE__)); \
    static void MT_CONCAT(mt_test_, __LINE__)()

#define CHECK(expr)                                                \
    do                                                             \
    {                                                              \
        ++::MicroTest::CheckCount();                               \
        if (!(expr))                                               \
            ::MicroTest::ReportFailure(__FILE__, __LINE__, #expr); \
    }                                                              \
    while (0)

#define CHECK_EQ(a, b)                                                    \
    do                                                                    \
    {                                                                     \
        ++::MicroTest::CheckCount();                                      \
        if (!((a) == (b)))                                                \
            ::MicroTest::ReportFailure(__FILE__, __LINE__, #a " == " #b); \
    }                                                                     \
    while (0)
