#pragma once

#include <CS2Kit/Menu/MenuOption.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace CS2Kit::Menu
{

struct Menu;

/**
 * Push a built submenu onto the player's menu stack. The factory is invoked lazily on E.
 *
 * The activation path lives in `MenuOption.cpp` to avoid pulling `MenuManager.hpp`
 * into this header (which would create a circular include via the manager → menu chain).
 */
class SubmenuOption : public MenuOption
{
public:
    using FactoryFn = std::function<std::shared_ptr<Menu>(int)>;

    SubmenuOption(std::string label, FactoryFn factory, bool enabled = true)
        : _label(std::move(label)), _factory(std::move(factory))
    {
        _enabled = enabled;
    }

    std::string GetLabel(int /*slot*/) const override { return _label; }
    void OnActivate(int slot) override;

private:
    std::string _label;
    FactoryFn _factory;
};

}  // namespace CS2Kit::Menu
