#pragma once

#include <cstdint>
#include <string>

namespace CS2Kit::Sdk
{

/**
 * Scan a loaded module's memory for a byte pattern (hex string with '?' wildcards).
 * Returns the first match address, or nullptr if not found.
 */
void* FindPattern(const char* moduleName, const std::string& pattern);

/**
 * Resolve a RIP-relative address: reads the 32-bit displacement at addr+ripOffset
 * and computes the absolute target as addr + ripOffset + ripSize + displacement.
 */
uintptr_t ResolveRelativeAddress(uintptr_t addr, int ripOffset, int ripSize);

}  // namespace CS2Kit::Sdk
