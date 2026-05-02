#include "Menu/MenuRenderer.hpp"

#include <CS2Kit/Menu/MenuOption.hpp>

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
    return html.str();
}

std::string DefaultFooter(bool isSubmenu, bool isPaginated)
{
    const char* closeColor = isSubmenu ? Theme::NavBack : Theme::NavClose;
    const char* closeLabel = isSubmenu ? "Back" : "Close";

    std::ostringstream html;
    html << "<font class='fontSize-s'>"
         << "<font color='" << Theme::NavGold << "'>[W/S]</font> "
         << "<font color='" << Theme::WarmGray << "'>Navigate</font>"
         << " · "
         << "<font color='" << Theme::Gold << "'>[E]</font> "
         << "<font color='" << Theme::WarmGray << "'>Select</font>";

    // When paginated there are four hint chunks — splitting onto two short rows is more reliable
    // than relying on the HUD's word wrap, which sometimes pushes [R] past the visible area.
    if (isPaginated)
    {
        html << "<br>"
             << "<font color='" << Theme::NavGold << "'>[A/D]</font> "
             << "<font color='" << Theme::WarmGray << "'>Page</font>"
             << " · "
             << "<font color='" << closeColor << "'>[R]</font> "
             << "<font color='" << Theme::WarmGray << "'>" << closeLabel << "</font>";
    }
    else
    {
        html << " · "
             << "<font color='" << closeColor << "'>[R]</font> "
             << "<font color='" << Theme::WarmGray << "'>" << closeLabel << "</font>";
    }

    html << "</font>";
    return html.str();
}

static std::string RenderItems(const Menu* menu, int slot, int selectedIndex, int pageStart, int pageEnd)
{
    std::ostringstream html;

    for (int i = pageStart; i < pageEnd; ++i)
    {
        const auto& opt = menu->Items[i];
        if (!opt)
            continue;

        std::string title = opt->GetLabel(slot);
        bool selectable = opt->IsSelectable();
        bool enabled = opt->IsEnabled();

        if (!enabled)
        {
            html << "<font color='" << Theme::Disabled << "'>- " << title << "</font><br>";
        }
        else if (!selectable)
        {
            // Rendered without a cursor glyph — the row is informational, not a target.
            html << "<font color='" << Theme::WarmGray << "'>" << title << "</font><br>";
        }
        else if (i == selectedIndex)
        {
            html << "<font color='" << Theme::Amber << "'><b>&gt; " << title << "</b></font> "
                 << "<font color='" << Theme::Gold << "'>[E]</font><br>";
        }
        else
        {
            html << "<font color='" << Theme::WarmWhite << "'>  " << title << "</font><br>";
        }
    }

    return html.str();
}

std::string RenderMenuHtml(const Menu* menu, int slot, int selectedIndex, bool isSubmenu)
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

    html << RenderItems(menu, slot, selectedIndex, pageStart, pageEnd);

    if (menu->Layout.Footer)
    {
        html << menu->Layout.Footer();
    }
    else
    {
        html << DefaultFooter(isSubmenu, totalPages > 1);
    }

    return html.str();
}

std::string RenderCaptureOverlay(const std::string& menuTitle, std::string_view prompt)
{
    std::ostringstream html;
    html << "<font color='" << Theme::Gold << "'><b>" << menuTitle << "</b></font><br>"
         << "<font color='" << Theme::WarmWhite << "'>" << prompt << "</font><br>"
         << "<font class='fontSize-s' color='" << Theme::WarmGray << "'>Type your answer in chat</font><br>"
         << "<font class='fontSize-s'>"
         << "<font color='" << Theme::NavClose << "'>[R]</font> "
         << "<font color='" << Theme::WarmGray << "'>Cancel</font>"
         << "</font>";
    return html.str();
}

}  // namespace CS2Kit::Menu
