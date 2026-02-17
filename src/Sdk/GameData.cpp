#include <CS2Kit/Sdk/GameData.hpp>

#include <CS2Kit/Core/Paths.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <CS2Kit/Sdk/SigScanner.hpp>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace CS2Kit::Sdk
{
using namespace CS2Kit::Utils;

bool GameData::Load(const std::string& path)
{
    try
    {
        auto fullPath = Core::ResolvePath(path);
        std::ifstream file(fullPath);
        if (!file.is_open())
        {
            Log::Warn("GameData file not found: {}", path);
            return false;
        }

        auto json = nlohmann::json::parse(file,
                                          /*cb=*/nullptr, /*allow_exceptions=*/true, /*ignore_comments=*/true);

#ifdef _WIN32
        constexpr const char* platform = "windows";
#else
        constexpr const char* platform = "linux";
#endif

        if (json.contains("offsets"))
        {
            for (auto& [name, entry] : json["offsets"].items())
            {
                if (entry.contains(platform))
                    _offsets[name] = entry[platform].get<int>();
            }
        }

        if (json.contains("signatures"))
        {
            for (auto& [name, entry] : json["signatures"].items())
            {
                if (!entry.contains(platform))
                    continue;

                auto& platEntry = entry[platform];
                SignatureEntry sig;
                sig.Library = entry.value("library", "server");
                sig.Pattern = platEntry.value("pattern", "");
                sig.Offset = platEntry.value("offset", 0);
                _signatures[name] = std::move(sig);
            }
        }

        Log::Info("GameData loaded: {} offsets, {} signatures.", _offsets.size(), _signatures.size());
        return true;
    }
    catch (const std::exception& e)
    {
        Log::Warn("Failed to parse GameData: {}", e.what());
        return false;
    }
}

int GameData::GetOffset(const std::string& name) const
{
    auto it = _offsets.find(name);
    return it != _offsets.end() ? it->second : -1;
}

void* GameData::FindSignature(const std::string& name) const
{
    auto it = _signatures.find(name);
    if (it == _signatures.end())
        return nullptr;

    auto& sig = it->second;
    return CS2Kit::Sdk::FindPattern(sig.Library.c_str(), sig.Pattern);
}

void* GameData::ResolveSignature(const std::string& name) const
{
    auto it = _signatures.find(name);
    if (it == _signatures.end())
        return nullptr;

    auto& sig = it->second;
    void* match = CS2Kit::Sdk::FindPattern(sig.Library.c_str(), sig.Pattern);
    if (!match)
        return nullptr;

    if (sig.Offset == 0)
        return match;

    auto addr = reinterpret_cast<uintptr_t>(match) + sig.Offset;
    addr = ResolveRelativeAddress(addr, 0, 4);
    if (addr == 0)
        return nullptr;
    return reinterpret_cast<void*>(addr);
}

}  // namespace CS2Kit::Sdk
