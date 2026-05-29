#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace CS2Kit::Utils
{

/**
 * @brief Localization system. Loads one JSON file per language; nested objects flatten into
 * dotted keys (`category.punish`). A @ref SlotScope routes @ref Get through a slot's language
 * (falling back to the active language, then English) without affecting code outside the scope.
 */
class Translations
{
public:
    static constexpr int MaxSlots = 64;

    Translations() = default;

    bool Load(const std::string& dirPath);
    void SetLanguage(const std::string& lang);
    const std::string& GetLanguage() const;
    std::string Get(const std::string& key) const;

    /** Language codes that were successfully loaded (one per JSON file). */
    std::vector<std::string> GetAvailableLanguages() const;

    /** Set/clear a slot's preferred language. Empty (or cleared) means "use the active language". */
    void SetPlayerLanguage(int slot, const std::string& lang);
    void ClearPlayerLanguage(int slot);

    /** RAII: route @ref Get through @p slot's language for the scope's lifetime. Nestable. */
    struct SlotScope
    {
        explicit SlotScope(int slot);
        ~SlotScope();
        SlotScope(const SlotScope&) = delete;
        SlotScope& operator=(const SlotScope&) = delete;

    private:
        int _prev;
    };

private:
    const std::string& ResolveLanguage() const;

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _translations;
    std::string _activeLang = "en";
    std::array<std::string, MaxSlots> _playerLangs{};
    int _currentSlot = -1;
};

}  // namespace CS2Kit::Utils
