#pragma once

#include <CS2Kit/Core/Slot.hpp>
#include <CS2Kit/Menu/Menu.hpp>
#include <array>

namespace CS2Kit::Menu
{

/**
 * @brief WASD-navigated center-HTML menus for all players.
 * Supports a per-player menu stack (submenus push, R pops back).
 * Driven by OnGameFrame() - reads button state each tick for input.
 */
class MenuManager
{
public:
    MenuManager() = default;

    /** Push @p menu onto the player's stack and start rendering it. */
    void OpenMenu(int slot, std::shared_ptr<MenuView> menu);

    /** Pop the top menu (firing its OnClose); falls back to the parent if one exists. */
    void CloseMenu(int slot);

    /** Clear the entire stack and hide the HUD for the player. */
    void CloseAllMenus(int slot);

    /** True if the player has any menu currently open. */
    bool HasActiveMenu(int slot) const;

    /**
     * @brief When enabled, the player's movement is frozen for as long as a menu is open,
     * so WASD navigation does not also walk the player around. The original MoveType is
     * restored when the last menu closes. Disabled by default.
     */
    void SetFreezePlayer(bool enabled) { _freezePlayer = enabled; }

    /** Per-tick driver: reads buttons, advances selection, and re-renders. */
    void OnGameFrame();

    /** Resets state for the disconnected slot so it cannot leak into a new player. */
    void OnPlayerDisconnect(int slot);

private:
    void HandleInput(int slot, uint64_t buttons, uint64_t prevButtons);
    void RenderMenu(int slot);

    /** Freeze (true) or restore (false) the player's movement; no-op unless freeze is enabled. */
    void SetPlayerFrozen(int slot, bool frozen);

    /** Per-player menu state, one entry per slot. */
    std::array<PlayerMenuState, Core::MaxPlayers> _states;
    static constexpr int64_t InputDebounceMs = 200;
    bool _freezePlayer = false;
};

}  // namespace CS2Kit::Menu
