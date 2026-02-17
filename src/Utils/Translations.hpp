#pragma once

#include "../Core/Singleton.hpp"

#include <string>
#include <unordered_map>

namespace CS2Kit::Utils
{

/**
 * Localization system. Loads JSON translation files from a directory (one per language),
 * and provides key-based string lookup for the active language.
 */
class Translations : public Singleton<Translations>
{
public:
    explicit Translations(Token) {}

    bool Load(const std::string& dirPath);
    void SetLanguage(const std::string& lang);
    const std::string& GetLanguage() const;
    std::string Get(const std::string& key) const;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _translations;
    std::string _activeLang = "en";
};

}  // namespace CS2Kit::Utils
