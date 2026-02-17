#pragma once

#include "IMenuIO.hpp"
#include "Menu.hpp"
#include "../Core/Singleton.hpp"
#include <array>

namespace CS2Kit::Menu
{

/**
 * Manages WASD-navigated center-HTML menus for all players.
 * Supports a per-player menu stack (submenus push, R pops back).
 * Driven by OnGameFrame() — reads button state each tick for input.
 */
class MenuManager : public Core::Singleton<MenuManager>
{
public:
    explicit MenuManager(Token) {}

    /** Set the menu I/O backend. Must be called before menus are used. */
    void SetMenuIO(IMenuIO* menuIO) { _menuIO = menuIO; }

    void OpenMenu(int slot, std::shared_ptr<Menu> menu);
    void CloseMenu(int slot);
    void CloseAllMenus(int slot);
    bool HasActiveMenu(int slot) const;
    void OnGameFrame();
    void OnPlayerDisconnect(int slot);

private:
    void HandleInput(int slot, uint64_t buttons, uint64_t prevButtons);
    void RenderMenu(int slot);

    IMenuIO* _menuIO = nullptr;
    std::array<PlayerMenuState, 64> _states;
    static constexpr int64_t InputDebounceMs = 200;
};

}  // namespace CS2Kit::Menu
