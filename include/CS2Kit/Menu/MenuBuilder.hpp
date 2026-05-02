#pragma once

#include <CS2Kit/Menu/Menu.hpp>
#include <CS2Kit/Menu/Options.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace CS2Kit::Menu
{

/**
 * @brief Fluent builder for constructing Menu instances.
 *
 * Each `Add*` method appends a typed @ref MenuOption to the menu. Plain action rows
 * use @ref AddButton; toggles, choice pickers, sliders, progress bars, free-text inputs,
 * and submenu links each have a dedicated builder method that constructs the matching
 * option subclass.
 *
 * Usage:
 * @code
 * MenuBuilder("Settings")
 *     .AddToggle("Beacon", "ON", "OFF", getFn, toggleFn)
 *     .AddChoice<int>("Speed", {{"Slow", 1}, {"Fast", 5}}, getIdx, setIdx, applyFn)
 *     .AddInput("Reason", "Type your reason in chat", getFn, setFn)
 *     .AddSubmenu("Advanced", &BuildAdvancedMenu)
 *     .Build();
 * @endcode
 */
class MenuBuilder
{
public:
    explicit MenuBuilder(const std::string& title) : _menu(std::make_shared<Menu>()) { _menu->Title = title; }

    /** Append a non-selectable label row (heading or divider). */
    MenuBuilder& AddText(const std::string& label)
    {
        _menu->Items.push_back(std::make_shared<TextOption>(label));
        return *this;
    }

    /** Append a plain action row. E fires the callback. */
    MenuBuilder& AddButton(const std::string& label, std::function<void(int)> onActivate, bool enabled = true)
    {
        _menu->Items.push_back(std::make_shared<ButtonOption>(label, std::move(onActivate), enabled));
        return *this;
    }

    /** Append an action row with a label that is recomputed every render. */
    MenuBuilder& AddDynamicButton(std::function<std::string()> getLabel, std::function<void(int)> onActivate,
                                  bool enabled = true)
    {
        _menu->Items.push_back(std::make_shared<ButtonOption>(std::move(getLabel), std::move(onActivate), enabled));
        return *this;
    }

    /** Append a toggle row. E and A/D both flip. State is read via @p getState every frame. */
    MenuBuilder& AddToggle(const std::string& title, const std::string& onLabel, const std::string& offLabel,
                           std::function<bool(int)> getState, std::function<void(int)> onToggle, bool enabled = true)
    {
        _menu->Items.push_back(std::make_shared<ToggleOption>(title, onLabel, offLabel, std::move(getState),
                                                              std::move(onToggle), enabled));
        return *this;
    }

    /** Append a string-labeled choice cycle. A/D walks the list; E commits the current value. */
    template <typename T>
    MenuBuilder& AddChoice(const std::string& title, std::vector<typename ChoiceOption<T>::Choice> choices,
                           std::function<int(int)> getIndex, std::function<void(int, int)> setIndex,
                           std::function<void(int, const T&)> onCommit = nullptr, bool enabled = true)
    {
        _menu->Items.push_back(std::make_shared<ChoiceOption<T>>(title, std::move(choices), std::move(getIndex),
                                                                 std::move(setIndex), std::move(onCommit), enabled));
        return *this;
    }

    /** Like @ref AddChoice but uses a formatter to derive labels from arbitrary values. */
    template <typename T>
    MenuBuilder& AddSelector(const std::string& title, std::vector<T> values,
                             std::function<std::string(const T&)> formatter, std::function<int(int)> getIndex,
                             std::function<void(int, int)> setIndex,
                             std::function<void(int, const T&)> onCommit = nullptr, bool enabled = true)
    {
        _menu->Items.push_back(std::make_shared<SelectorOption<T>>(title, std::move(values), std::move(formatter),
                                                                   std::move(getIndex), std::move(setIndex),
                                                                   std::move(onCommit), enabled));
        return *this;
    }

    /** Append a numeric slider. A/D adjusts in `step` units, clamped to `[min, max]`. */
    MenuBuilder& AddSlider(const std::string& title, int min, int max, int step, std::function<int(int)> getValue,
                           std::function<void(int, int)> setValue, bool enabled = true)
    {
        _menu->Items.push_back(
            std::make_shared<SliderOption>(title, min, max, step, std::move(getValue), std::move(setValue), enabled));
        return *this;
    }

    /** Append a read-only progress bar. */
    MenuBuilder& AddProgressBar(const std::string& title, std::function<int(int)> getValue, int max)
    {
        _menu->Items.push_back(std::make_shared<ProgressBarOption>(title, std::move(getValue), max));
        return *this;
    }

    /**
     * Append a free-text input row. E starts a chat capture; the player's next chat
     * line is routed to @p set. Return false from @p set to re-prompt for invalid input.
     */
    MenuBuilder& AddInput(const std::string& title, const std::string& prompt, std::function<std::string(int)> get,
                          std::function<bool(int, std::string_view)> set, int maxLength = 64, bool enabled = true)
    {
        _menu->Items.push_back(
            std::make_shared<InputOption>(title, prompt, std::move(get), std::move(set), maxLength, enabled));
        return *this;
    }

    /** Append a submenu link. E builds and pushes the submenu via @p factory. */
    MenuBuilder& AddSubmenu(const std::string& label, std::function<std::shared_ptr<Menu>(int)> factory,
                            bool enabled = true)
    {
        _menu->Items.push_back(std::make_shared<SubmenuOption>(label, std::move(factory), enabled));
        return *this;
    }

    /** Escape hatch: append a user-defined option subclass. */
    MenuBuilder& AddOption(std::shared_ptr<MenuOption> option)
    {
        _menu->Items.push_back(std::move(option));
        return *this;
    }

    /** Set a callback invoked with the player slot when the menu is dismissed. */
    MenuBuilder& OnClose(std::function<void(int)> callback)
    {
        _menu->OnClose = std::move(callback);
        return *this;
    }

    /** Override the default title + page-indicator header with custom HTML. */
    MenuBuilder& WithHeader(std::function<std::string()> header)
    {
        _menu->Layout.Header = std::move(header);
        return *this;
    }

    /** Override the default key-hints footer with custom HTML. */
    MenuBuilder& WithFooter(std::function<std::string()> footer)
    {
        _menu->Layout.Footer = std::move(footer);
        return *this;
    }

    /** Finalize and return the built menu. The builder must not be reused after this. */
    std::shared_ptr<Menu> Build() { return std::move(_menu); }

private:
    std::shared_ptr<Menu> _menu;
};

}  // namespace CS2Kit::Menu
