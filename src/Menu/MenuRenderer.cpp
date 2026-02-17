#include "Menu/MenuRenderer.hpp"

#include <sstream>

namespace CS2Kit::Menu
{

namespace Theme
{
constexpr const char* Gold = "#FFD700";
constexpr const char* Amber = "#FF8C00";
constexpr const char* WarmWhite = "#CCBBAA";
constexpr const char* WarmGray = "#887755";
constexpr const char* Disabled = "#665544";
constexpr const char* NavGold = "#AA8833";
constexpr const char* NavClose = "#AA4422";
}  // namespace Theme

std::string DefaultHeader(const std::string& title)
{
    std::ostringstream html;
    html << "<font color='" << Theme::Gold << "'><b>" << title << "</b></font><br>";
    html << "<font class='fontSize-s' color='" << Theme::Gold << "'>──────────────────────────</font><br>";
    return html.str();
}

std::string DefaultFooter()
{
    std::ostringstream html;
    html << "<font class='fontSize-s' color='" << Theme::Gold << "'>──────────────────────────</font><br>";
    html << "<font class='fontSize-sm'>"
         << "<font color='" << Theme::NavGold << "'>[W/S]</font> "
         << "<font color='" << Theme::WarmGray << "'>Navigate</font>"
         << " · "
         << "<font color='" << Theme::Gold << "'>[E]</font> "
         << "<font color='" << Theme::WarmGray << "'>Select</font>"
         << " · "
         << "<font color='" << Theme::NavClose << "'>[R]</font> "
         << "<font color='" << Theme::WarmGray << "'>Close</font>"
         << "</font>";
    return html.str();
}

static std::string RenderItems(const Menu* menu, int selectedIndex)
{
    std::ostringstream html;

    for (int i = 0; i < static_cast<int>(menu->Items.size()); ++i)
    {
        const auto& item = menu->Items[i];

        if (!item.Enabled)
        {
            html << "<font color='" << Theme::Disabled << "'>- " << item.Title << "</font><br>";
        }
        else if (i == selectedIndex)
        {
            html << "<font color='" << Theme::Amber << "'><b>&gt; " << item.Title << "</b></font> "
                 << "<font color='" << Theme::Gold << "'>[E]</font><br>";
        }
        else
        {
            html << "<font color='" << Theme::WarmWhite << "'>  " << item.Title << "</font><br>";
        }
    }

    return html.str();
}

std::string RenderMenuHtml(const Menu* menu, int selectedIndex)
{
    if (!menu)
        return "";

    std::ostringstream html;

    if (menu->Layout.Header)
    {
        html << menu->Layout.Header();
    }
    else
    {
        html << DefaultHeader(menu->Title);
    }

    html << RenderItems(menu, selectedIndex);

    if (menu->Layout.Footer)
    {
        html << menu->Layout.Footer();
    }
    else
    {
        html << DefaultFooter();
    }

    return html.str();
}

}  // namespace CS2Kit::Menu
