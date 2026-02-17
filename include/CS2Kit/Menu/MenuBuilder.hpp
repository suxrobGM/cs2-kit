#pragma once

#include <CS2Kit/Menu/Menu.hpp>

namespace CS2Kit::Menu
{

/**
 * Fluent builder for constructing Menu instances.
 * Usage: MenuBuilder("Title").AddItem("Foo", handler).AddItem("Bar", handler, false).Build()
 */
class MenuBuilder
{
public:
    explicit MenuBuilder(const std::string& title) : _menu(std::make_shared<Menu>()) { _menu->Title = title; }

    MenuBuilder& AddItem(const std::string& title, std::function<void(int)> onSelect)
    {
        _menu->Items.push_back({.Title = title, .OnSelect = std::move(onSelect)});
        return *this;
    }

    MenuBuilder& AddItem(const std::string& title, std::function<void(int)> onSelect, bool enabled)
    {
        _menu->Items.push_back({.Title = title, .OnSelect = std::move(onSelect), .Enabled = enabled});
        return *this;
    }

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
                    {
                    }
                },
            .Enabled = enabled,
        });
        return *this;
    }

    MenuBuilder& OnClose(std::function<void(int)> callback)
    {
        _menu->OnClose = std::move(callback);
        return *this;
    }

    MenuBuilder& WithHeader(std::function<std::string()> header)
    {
        _menu->Layout.Header = std::move(header);
        return *this;
    }

    MenuBuilder& WithFooter(std::function<std::string()> footer)
    {
        _menu->Layout.Footer = std::move(footer);
        return *this;
    }

    std::shared_ptr<Menu> Build() { return std::move(_menu); }

private:
    std::shared_ptr<Menu> _menu;
};

}  // namespace CS2Kit::Menu
