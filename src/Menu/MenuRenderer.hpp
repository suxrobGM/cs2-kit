#pragma once

#include <CS2Kit/Menu/Menu.hpp>
#include <string>

namespace CS2Kit::Menu
{
/** Renders the HTML for a menu, including its items and layout. */
std::string RenderMenuHtml(const Menu* menu, int selectedIndex, bool isSubmenu);

/** Generates the default header HTML for a menu. */
std::string DefaultHeader(const std::string& title, int currentPage, int totalPages);

/** Generates the default footer HTML for a menu. */
std::string DefaultFooter(bool isSubmenu);

}  // namespace CS2Kit::Menu
