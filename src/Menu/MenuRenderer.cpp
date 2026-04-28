#include "Menu/MenuRenderer.hpp"

#include <algorithm>
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
constexpr const char* NavBack = "#AA8833";
}  // namespace Theme

std::string DefaultHeader(const std::string& title, int currentPage, int totalPages)
{
    std::ostringstream html;
    html << "<font color='" << Theme::Gold << "'><b>" << title << "</b></font>";

    if (totalPages > 1)
    {
        html << " <font class='fontSize-s' color='" << Theme::WarmGray << "'>(" << (currentPage + 1) << "/"
             << totalPages << ")</font>";
    }

    html << "<br>";
    html << "<font class='fontSize-s' color='" << Theme::Gold << "'>──────────────────────────</font><br>";
    return html.str();
}

std::string DefaultFooter(bool isSubmenu)
{
    std::ostringstream html;
    html << "<font class='fontSize-s' color='" << Theme::Gold << "'>──────────────────────────</font><br>";
    html << "<font class='fontSize-sm'>"
         << "<font color='" << Theme::NavGold << "'>[W/S]</font> "
         << "<font color='" << Theme::WarmGray << "'>Navigate</font>"
         << " · "
         << "<font color='" << Theme::Gold << "'>[E]</font> "
         << "<font color='" << Theme::WarmGray << "'>Select</font>"
         << " · ";

    if (isSubmenu)
    {
        html << "<font color='" << Theme::NavBack << "'>[R]</font> "
             << "<font color='" << Theme::WarmGray << "'>Back</font>";
    }
    else
    {
        html << "<font color='" << Theme::NavClose << "'>[R]</font> "
             << "<font color='" << Theme::WarmGray << "'>Close</font>";
    }

    html << "</font>";
    return html.str();
}

static std::string RenderItems(const Menu* menu, int selectedIndex, int pageStart, int pageEnd)
{
    std::ostringstream html;

    for (int i = pageStart; i < pageEnd; ++i)
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

std::string RenderMenuHtml(const Menu* menu, int selectedIndex, bool isSubmenu)
{
    if (!menu)
    {
        return "";
    }

    int itemCount = static_cast<int>(menu->Items.size());
    int totalPages = itemCount == 0 ? 1 : (itemCount + ItemsPerPage - 1) / ItemsPerPage;
    int currentPage = itemCount == 0 ? 0 : selectedIndex / ItemsPerPage;
    int pageStart = currentPage * ItemsPerPage;
    int pageEnd = std::min(itemCount, pageStart + ItemsPerPage);

    std::ostringstream html;

    if (menu->Layout.Header)
    {
        html << menu->Layout.Header();
    }
    else
    {
        html << DefaultHeader(menu->Title, currentPage, totalPages);
    }

    html << RenderItems(menu, selectedIndex, pageStart, pageEnd);

    if (menu->Layout.Footer)
    {
        html << menu->Layout.Footer();
    }
    else
    {
        html << DefaultFooter(isSubmenu);
    }

    return html.str();
}

}  // namespace CS2Kit::Menu
