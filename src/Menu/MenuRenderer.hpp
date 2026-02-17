#pragma once

#include <CS2Kit/Menu/Menu.hpp>
#include <string>

namespace CS2Kit::Menu
{

std::string RenderMenuHtml(const Menu* menu, int selectedIndex);
std::string DefaultHeader(const std::string& title);
std::string DefaultFooter();

}  // namespace CS2Kit::Menu
