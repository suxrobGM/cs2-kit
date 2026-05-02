#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace CS2Kit::Menu
{

class MenuOption;

/** Maximum items shown per page before the menu paginates. */
inline constexpr int ItemsPerPage = 5;

/** Optional custom HTML providers for the header and footer regions of a menu. */
struct MenuLayout
{
    /** Replaces the default title + page indicator block. */
    std::function<std::string()> Header;
    /** Replaces the default key-hints block (W/S, E, R). */
    std::function<std::string()> Footer;
};

/** A WASD-navigable menu rendered as center-HTML. Build with MenuBuilder. */
struct Menu
{
    std::string Title;
    std::vector<std::shared_ptr<MenuOption>> Items;
    /** Invoked with the player slot when the menu is dismissed (R pressed or popped). */
    std::function<void(int)> OnClose;
    MenuLayout Layout;
};

/**
 * Per-player menu runtime state held by MenuManager. The stack supports
 * submenus: opening pushes, R or programmatic close pops back to the parent.
 */
struct PlayerMenuState
{
    /** The stack of menus currently open for the player. */
    std::stack<std::shared_ptr<Menu>> MenuStack;
    int SelectedIndex = 0;
    int64_t LastInputTime = 0;
    uint64_t PrevButtons = 0;

    /** True if the player has any menu currently open. */
    bool HasMenu() const { return !MenuStack.empty(); }
    /** Top of the stack, or nullptr if no menu is open. */
    Menu* GetCurrentMenu() { return MenuStack.empty() ? nullptr : MenuStack.top().get(); }

    /** Clears the entire menu stack and resets selection/input state. */
    void Reset()
    {
        while (!MenuStack.empty())
        {
            MenuStack.pop();
        }

        SelectedIndex = 0;
        LastInputTime = 0;
        PrevButtons = 0;
    }
};

}  // namespace CS2Kit::Menu
