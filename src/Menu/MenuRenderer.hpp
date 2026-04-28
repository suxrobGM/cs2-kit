#pragma once

#include <CS2Kit/Menu/Menu.hpp>
#include <string>

namespace CS2Kit::Menu
{
/** Renders the HTML for a menu, including its items and layout. */
std::string RenderMenuHtml(const Menu* menu, int selectedIndex, bool isSubmenu);

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
