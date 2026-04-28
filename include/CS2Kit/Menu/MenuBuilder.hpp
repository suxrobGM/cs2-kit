#pragma once

#include <CS2Kit/Menu/Menu.hpp>

namespace CS2Kit::Menu
{

/**
 * @brief Fluent builder for constructing Menu instances.
 * Usage: MenuBuilder("Title").AddItem("Foo", handler).AddItem("Bar", handler, false).Build()
 */
class MenuBuilder
{
public:
    /** Start a new builder with the given menu title. */
    explicit MenuBuilder(const std::string& title) : _menu(std::make_shared<Menu>()) { _menu->Title = title; }

    /** Append a selectable item; @p onSelect receives the player slot when activated. */
    MenuBuilder& AddItem(const std::string& title, std::function<void(int)> onSelect)
    {
        _menu->Items.push_back({.Title = title, .OnSelect = std::move(onSelect)});
        return *this;
    }

    /** Append an item with an explicit enabled flag (disabled items are greyed out and skipped). */
    MenuBuilder& AddItem(const std::string& title, std::function<void(int)> onSelect, bool enabled)
    {
        _menu->Items.push_back({.Title = title, .OnSelect = std::move(onSelect), .Enabled = enabled});
        return *this;
    }

    /**
     * Append an item whose label is recomputed every render via @p onGetTitle.
     * Use for toggles where the row should reflect live state — e.g. "Beacon: ON/OFF".
     */
    MenuBuilder& AddDynamicItem(std::function<std::string()> onGetTitle, std::function<void(int)> onSelect,
                                bool enabled = true)
    {
        _menu->Items.push_back({
            .Title = "",
            .OnGetTitle = std::move(onGetTitle),
            .OnSelect = std::move(onSelect),
            .Enabled = enabled,
        });
        return *this;
    }

    /** Append an item that lazily builds a submenu via @p factory when activated. */
    MenuBuilder& AddSubmenu(const std::string& title, std::function<std::shared_ptr<Menu>(int)> factory,
                            bool enabled = true)
    {
        auto fn = std::move(factory);
        _menu->Items.push_back({
            .Title = title,
            .OnSelect =
                [fn](int slot) {
                    auto submenu = fn(slot);
                    if (submenu)
                    {}
                },
            .Enabled = enabled,
        });
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
