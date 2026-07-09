#pragma once

#include <cstdint>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace CS2Kit::Sdk
{

struct ScanResult
{
    void* Address = nullptr;  // first match, or nullptr
    bool Unique = true;       // false when the pattern matched more than once
};

/**
 * Scan a loaded module's memory for a byte pattern (hex string with '?' wildcards).
 * Keeps scanning after the first hit so an ambiguous pattern is reported, not
 * silently taken.
 */
ScanResult FindPatternEx(const char* moduleName, const std::string& pattern);

/** First-match convenience wrapper over FindPatternEx. */
void* FindPattern(const char* moduleName, const std::string& pattern);

/**
 * Resolve a RIP-relative address: reads the 32-bit displacement at addr+ripOffset
 * and computes the absolute target as addr + ripOffset + ripSize + displacement.
 */
uintptr_t ResolveRelativeAddress(uintptr_t addr, int ripOffset, int ripSize);

}  // namespace CS2Kit::Sdk
