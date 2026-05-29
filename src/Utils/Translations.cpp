#include <CS2Kit/Core/Paths.hpp>
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <CS2Kit/Utils/Translations.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace CS2Kit::Utils
{

namespace
{
// Flatten nested objects into dotted keys (`category.punish`), so callers can group keys in
// the JSON without changing the flat lookup model. Leaf string values are stored; non-string
// leaves are ignored.
void FlattenInto(const nlohmann::json& node, const std::string& prefix,
                 std::unordered_map<std::string, std::string>& out)
{
    for (auto& [key, value] : node.items())
    {
        std::string full = prefix.empty() ? key : prefix + "." + key;
        if (value.is_object())
            FlattenInto(value, full, out);
        else if (value.is_string())
            out[full] = value.get<std::string>();
    }
}
}  // namespace

bool Translations::Load(const std::string& dirPath)
{
    _translations.clear();
    namespace fs = std::filesystem;

    auto resolvedPath = Core::ResolvePath(dirPath);

    if (!fs::exists(resolvedPath) || !fs::is_directory(resolvedPath))
    {
        Log::Warn("Translations directory not found: {}", resolvedPath.string());
        return false;
    }

    int loaded = 0;
    for (const auto& entry : fs::directory_iterator(resolvedPath))
    {
        if (entry.path().extension() != ".json")
            continue;

        std::string langCode = entry.path().stem().string();
        try
        {
            std::ifstream file(entry.path());
            if (!file.is_open())
                continue;

            auto data = nlohmann::json::parse(file);
            auto& langMap = _translations[langCode];
            FlattenInto(data, "", langMap);

            ++loaded;
            Log::Info("Loaded translations: {} ({} keys)", langCode, langMap.size());
        }
        catch (const std::exception& e)
        {
            Log::Warn("Failed to parse {}: {}", entry.path().string(), e.what());
        }
    }

    Log::Info("Loaded {} language(s).", loaded);
    return loaded > 0;
}

void Translations::SetLanguage(const std::string& lang)
{
    _activeLang = lang;
}

const std::string& Translations::GetLanguage() const
{
    return _activeLang;
}

std::vector<std::string> Translations::GetAvailableLanguages() const
{
    std::vector<std::string> langs;
    langs.reserve(_translations.size());
    for (const auto& [code, _] : _translations)
    {
        langs.push_back(code);
    }

    return langs;
}

void Translations::SetPlayerLanguage(int slot, const std::string& lang)
{
    if (slot >= 0 && slot < MaxSlots)
    {
        _playerLangs[slot] = lang;
    }
}

void Translations::ClearPlayerLanguage(int slot)
{
    if (slot >= 0 && slot < MaxSlots)
    {
        _playerLangs[slot].clear();
    }
}

const std::string& Translations::ResolveLanguage() const
{
    if (_currentSlot >= 0 && _currentSlot < MaxSlots && !_playerLangs[_currentSlot].empty())
    {
        return _playerLangs[_currentSlot];
    }

    return _activeLang;
}

Translations::SlotScope::SlotScope(int slot) : _prev(CS2Kit::Core::Kit().Translations._currentSlot)
{
    CS2Kit::Core::Kit().Translations._currentSlot = slot;
}

Translations::SlotScope::~SlotScope()
{
    CS2Kit::Core::Kit().Translations._currentSlot = _prev;
}

std::string Translations::Get(const std::string& key) const
{
    const std::string& lang = ResolveLanguage();

    auto langIt = _translations.find(lang);
    if (langIt != _translations.end())
    {
        auto keyIt = langIt->second.find(key);
        if (keyIt != langIt->second.end())
        {
            return keyIt->second;
        }
    }

    if (lang != "en")
    {
        langIt = _translations.find("en");
        if (langIt != _translations.end())
        {
            auto keyIt = langIt->second.find(key);
            if (keyIt != langIt->second.end())
            {
                return keyIt->second;
            }
        }
    }

    return key;
}

}  // namespace CS2Kit::Utils
