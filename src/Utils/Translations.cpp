#include <CS2Kit/Utils/Translations.hpp>

#include <CS2Kit/Core/Paths.hpp>
#include <CS2Kit/Utils/Log.hpp>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace CS2Kit::Utils
{

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
            for (auto& [key, value] : data.items())
            {
                if (value.is_string())
                    langMap[key] = value.get<std::string>();
            }

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

std::string Translations::Get(const std::string& key) const
{
    auto langIt = _translations.find(_activeLang);
    if (langIt != _translations.end())
    {
        auto keyIt = langIt->second.find(key);
        if (keyIt != langIt->second.end())
            return keyIt->second;
    }

    if (_activeLang != "en")
    {
        langIt = _translations.find("en");
        if (langIt != _translations.end())
        {
            auto keyIt = langIt->second.find(key);
            if (keyIt != langIt->second.end())
                return keyIt->second;
        }
    }

    return key;
}

}  // namespace CS2Kit::Utils
