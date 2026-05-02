#pragma once

#include <CS2Kit/Menu/Menu.hpp>
#include <string>
#include <string_view>

namespace CS2Kit::Menu
{
/** Renders the HTML for a menu, including its items and layout. */
std::string RenderMenuHtml(const Menu* menu, int slot, int selectedIndex, bool isSubmenu);

/** Renders the chat-input capture overlay shown while a player is typing a value. */
std::string RenderCaptureOverlay(const std::string& menuTitle, std::string_view prompt);

/** Generates the default header HTML for a menu. */
std::string DefaultHeader(const std::string& title, int currentPage, int totalPages);

/**
 * Generates the default footer HTML for a menu.
 * @param isSubmenu True if this menu is a submenu (shows "Back" hint), false if it's a root menu (shows "Close" hint).
 * @param isPaginated True if the menu has multiple pages of items (shows page navigation hints)
 * @return The generated HTML string for the menu footer.
 */
std::string DefaultFooter(bool isSubmenu, bool isPaginated);

}  // namespace CS2Kit::Menu
