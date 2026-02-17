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

static bool GetModuleInfo(const char* moduleName, uint8_t*& base, size_t& size)
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

    base = static_cast<uint8_t*>(modInfo.lpBaseOfDll);
    size = modInfo.SizeOfImage;
    return true;
}

#else

struct ModuleInfo
{
    const char* name;
    uint8_t* base;
    size_t size;
    bool found;
};

static int DlIterateCallback(struct dl_phdr_info* info, size_t /*size*/, void* data)
{
    auto* mod = static_cast<ModuleInfo*>(data);
    if (info->dlpi_name && strstr(info->dlpi_name, mod->name))
    {
        mod->base = reinterpret_cast<uint8_t*>(info->dlpi_addr);
        size_t maxAddr = 0;
        for (int i = 0; i < info->dlpi_phnum; ++i)
        {
            if (info->dlpi_phdr[i].p_type == PT_LOAD)
            {
                size_t segEnd = info->dlpi_phdr[i].p_vaddr + info->dlpi_phdr[i].p_memsz;
                if (segEnd > maxAddr)
                    maxAddr = segEnd;
            }
        }
        mod->size = maxAddr;
        mod->found = true;
        return 1;
    }
    return 0;
}

static bool GetModuleInfo(const char* moduleName, uint8_t*& base, size_t& size)
{
    ModuleInfo mod{moduleName, nullptr, 0, false};
    dl_iterate_phdr(DlIterateCallback, &mod);
    if (mod.found)
    {
        base = mod.base;
        size = mod.size;
    }
    return mod.found;
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

    uint8_t* base = nullptr;
    size_t size = 0;
    if (!GetModuleInfo(fullName.c_str(), base, size))
    {
        Log::Error("SigScanner: Module '{}' not found.", fullName);
        return nullptr;
    }

    Log::Info("SigScanner: Scanning '{}' (base={:#x}, size=0x{:X})...", fullName, reinterpret_cast<uintptr_t>(base),
              size);

    auto patternBytes = ParsePattern(pattern);
    void* result = ScanMemory(base, size, patternBytes);
    if (!result)
    {
        Log::Warn("SigScanner: Pattern not found in '{}'.", fullName);
    }
    else
    {
        Log::Info("SigScanner: Found match at {:#x}.", reinterpret_cast<uintptr_t>(result));
    }
    return result;
}

uintptr_t ResolveRelativeAddress(uintptr_t addr, int ripOffset, int ripSize)
{
    if (addr == 0)
        return 0;

    int32_t relative = *reinterpret_cast<int32_t*>(addr + ripOffset);
    return addr + ripSize + relative;
}

}  // namespace CS2Kit::Sdk
