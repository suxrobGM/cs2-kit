#include "Sdk/SigScanner.hpp"

#include <CS2Kit/Utils/Log.hpp>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <psapi.h>
#else
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#endif

namespace CS2Kit::Sdk
{

using namespace CS2Kit::Utils;

struct PatternByte
{
    uint8_t value;
    bool wildcard;
};

// A mapped region to scan: the whole image on Windows, one PT_LOAD segment on Linux (so we never
// read across an unmapped `-z separate-code` gap).
struct ScanRange
{
    const uint8_t* base;
    size_t size;
};

static std::vector<PatternByte> ParsePattern(const std::string& pattern)
{
    std::vector<PatternByte> bytes;
    std::istringstream stream(pattern);
    std::string token;

    while (stream >> token)
    {
        if (token == "?" || token == "??")
        {
            bytes.push_back({0, true});
        }
        else
        {
            bytes.push_back({static_cast<uint8_t>(std::stoul(token, nullptr, 16)), false});
        }
    }
    return bytes;
}

static void* ScanMemory(const uint8_t* base, size_t size, const std::vector<PatternByte>& pattern)
{
    if (pattern.empty() || size < pattern.size())
        return nullptr;

    size_t scanEnd = size - pattern.size();
    for (size_t i = 0; i <= scanEnd; ++i)
    {
        bool found = true;
        for (size_t j = 0; j < pattern.size(); ++j)
        {
            if (!pattern[j].wildcard && base[i + j] != pattern[j].value)
            {
                found = false;
                break;
            }
        }
        if (found)
            return const_cast<uint8_t*>(base + i);
    }
    return nullptr;
}

#ifdef _WIN32

static bool GetScanRanges(const char* moduleName, std::vector<ScanRange>& ranges)
{
    HANDLE hProcess = GetCurrentProcess();
    HMODULE hModules[1024];
    DWORD cbNeeded = 0;

    if (!EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded))
        return false;

    DWORD moduleCount = cbNeeded / sizeof(HMODULE);
    HMODULE bestModule = nullptr;
    DWORD bestSize = 0;

    for (DWORD i = 0; i < moduleCount; ++i)
    {
        char modPath[MAX_PATH];
        if (!GetModuleFileNameA(hModules[i], modPath, sizeof(modPath)))
            continue;

        const char* fileName = strrchr(modPath, '\\');
        if (!fileName)
            fileName = strrchr(modPath, '/');
        fileName = fileName ? fileName + 1 : modPath;

        if (_stricmp(fileName, moduleName) != 0)
            continue;

        MODULEINFO modInfo{};
        if (GetModuleInformation(hProcess, hModules[i], &modInfo, sizeof(modInfo)))
        {
            if (modInfo.SizeOfImage > bestSize)
            {
                bestModule = hModules[i];
                bestSize = modInfo.SizeOfImage;
            }
        }
    }

    if (!bestModule)
        return false;

    MODULEINFO modInfo{};
    if (!GetModuleInformation(hProcess, bestModule, &modInfo, sizeof(modInfo)))
        return false;

    ranges.push_back({static_cast<const uint8_t*>(modInfo.lpBaseOfDll), modInfo.SizeOfImage});
    return true;
}

#else

static const char* BaseName(const char* path)
{
    const char* slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

struct ModuleScan
{
    const char* name;             // basename to match, e.g. "libserver.so"
    size_t bestSpan;              // largest module span seen so far (selects the real lib)
    std::vector<ScanRange> ranges;  // PT_LOAD segments of the selected module
};

// Multiple objects can share the basename "libserver.so" (a small loader stub plus the real game
// library); a substring + first-match scan picked the stub. Match the exact basename and keep the
// largest-span mapping, recording its PT_LOAD segments.
static int DlIterateCallback(struct dl_phdr_info* info, size_t /*size*/, void* data)
{
    auto* mod = static_cast<ModuleScan*>(data);
    if (!info->dlpi_name || strcmp(BaseName(info->dlpi_name), mod->name) != 0)
        return 0;

    size_t span = 0;
    std::vector<ScanRange> segments;
    for (int i = 0; i < info->dlpi_phnum; ++i)
    {
        const auto& phdr = info->dlpi_phdr[i];
        if (phdr.p_type != PT_LOAD || phdr.p_memsz == 0)
            continue;
        size_t segEnd = phdr.p_vaddr + phdr.p_memsz;
        if (segEnd > span)
            span = segEnd;
        segments.push_back(
            {reinterpret_cast<const uint8_t*>(info->dlpi_addr + phdr.p_vaddr), phdr.p_memsz});
    }

    if (span > mod->bestSpan)
    {
        mod->bestSpan = span;
        mod->ranges = std::move(segments);
    }
    return 0;  // keep iterating; the largest match wins
}

static bool GetScanRanges(const char* moduleName, std::vector<ScanRange>& ranges)
{
    ModuleScan mod{moduleName, 0, {}};
    dl_iterate_phdr(DlIterateCallback, &mod);
    if (mod.ranges.empty())
        return false;
    ranges = std::move(mod.ranges);
    return true;
}

#endif

void* FindPattern(const char* moduleName, const std::string& pattern)
{
    std::string fullName;
#ifdef _WIN32
    fullName = std::string(moduleName) + ".dll";
#else
    fullName = std::string("lib") + moduleName + ".so";
#endif

    std::vector<ScanRange> ranges;
    if (!GetScanRanges(fullName.c_str(), ranges))
    {
        Log::Error("SigScanner: Module '{}' not found.", fullName);
        return nullptr;
    }

    size_t totalSize = 0;
    for (const auto& range : ranges)
        totalSize += range.size;
    Log::Info("SigScanner: Scanning '{}' ({} segment(s), 0x{:X} bytes)...", fullName, ranges.size(), totalSize);

    auto patternBytes = ParsePattern(pattern);
    for (const auto& range : ranges)
    {
        if (void* result = ScanMemory(range.base, range.size, patternBytes))
        {
            Log::Info("SigScanner: Found match at {:#x}.", reinterpret_cast<uintptr_t>(result));
            return result;
        }
    }

    Log::Warn("SigScanner: Pattern not found in '{}'.", fullName);
    return nullptr;
}

uintptr_t ResolveRelativeAddress(uintptr_t addr, int ripOffset, int ripSize)
{
    if (addr == 0)
        return 0;

    int32_t relative = *reinterpret_cast<int32_t*>(addr + ripOffset);
    return addr + ripSize + relative;
}

}  // namespace CS2Kit::Sdk
