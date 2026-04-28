#pragma once

#include <CS2Kit/Core/Singleton.hpp>
#include <CS2Kit/Menu/Menu.hpp>
#include <array>

namespace CS2Kit::Menu
{

/**
 * @brief WASD-navigated center-HTML menus for all players.
 * Supports a per-player menu stack (submenus push, R pops back).
 * Driven by OnGameFrame() — reads button state each tick for input.
 */
class MenuManager : public Core::Singleton<MenuManager>
{
public:
    explicit MenuManager(Token) {}

    /** Push @p menu onto the player's stack and start rendering it. */
    void OpenMenu(int slot, std::shared_ptr<Menu> menu);

    /** Pop the top menu (firing its OnClose); falls back to the parent if one exists. */
    void CloseMenu(int slot);

    /** Clear the entire stack and hide the HUD for the player. */
    void CloseAllMenus(int slot);

    /** True if the player has any menu currently open. */
    bool HasActiveMenu(int slot) const;

    /** Per-tick driver: reads buttons, advances selection, and re-renders. */
    void OnGameFrame();

    /** Resets state for the disconnected slot so it cannot leak into a new player. */
    void OnPlayerDisconnect(int slot);

private:
    void HandleInput(int slot, uint64_t buttons, uint64_t prevButtons);
    void RenderMenu(int slot);

    /** Per-player menu state. Max 64 players. */
    std::array<PlayerMenuState, 64> _states;
    static constexpr int64_t InputDebounceMs = 200;
};

}  // namespace CS2Kit::Menu
