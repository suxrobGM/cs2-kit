#include "Menu/MenuRenderer.hpp"

#include <CS2Kit/Menu/MenuManager.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <algorithm>
#include <chrono>

namespace CS2Kit::Menu
{

using namespace CS2Kit::Utils;
using namespace CS2Kit::Sdk;

static int64_t GetCurrentTimeMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

namespace
{

// Step the cursor by `step` (typically ±1), wrapping over the full item list and skipping disabled items.
void StepCursor(const std::vector<MenuItem>& items, int& idx, int step)
{
    int n = static_cast<int>(items.size());
    int attempts = n;
    do
    {
        idx = ((idx + step) % n + n) % n;
    }
    while (!items[idx].Enabled && --attempts > 0);
}

// Jump by `pageDelta` pages, preserving the in-page offset, then skip forward over disabled items within the new page.
void JumpPage(const std::vector<MenuItem>& items, int& idx, int pageDelta)
{
    int n = static_cast<int>(items.size());

    int pageCount = (n + ItemsPerPage - 1) / ItemsPerPage;
    int currentPage = idx / ItemsPerPage;
    int offset = idx % ItemsPerPage;
    int newPage = ((currentPage + pageDelta) % pageCount + pageCount) % pageCount;

    int pageStart = newPage * ItemsPerPage;
    int pageEnd = std::min(n, pageStart + ItemsPerPage);

    idx = std::min(pageStart + offset, pageEnd - 1);
    int attempts = pageEnd - pageStart;
    while (!items[idx].Enabled && --attempts > 0)
    {
        idx = (idx + 1 < pageEnd) ? idx + 1 : pageStart;
    }
}

}  // namespace

void MenuManager::OpenMenu(int slot, std::shared_ptr<Menu> menu)
{
    if (slot < 0 || slot >= 64 || !menu)
        return;

    auto& state = _states[slot];
    state.MenuStack.push(std::move(menu));
    state.SelectedIndex = 0;
    state.LastInputTime = GetCurrentTimeMs();

    auto* current = state.GetCurrentMenu();
    if (current)
    {
        Log::Info("Menu opened for slot {} (title: {}, items: {})", slot, current->Title, current->Items.size());
    }
}

void MenuManager::CloseMenu(int slot)
{
    if (slot < 0 || slot >= 64)
        return;

    auto& state = _states[slot];
    if (state.MenuStack.empty())
        return;

    auto menu = state.MenuStack.top();
    state.MenuStack.pop();

    if (menu->OnClose)
        menu->OnClose(slot);

    if (state.MenuStack.empty())
    {
        MessageSystem::Instance().ClearCenterHtml(slot);
        state.Reset();
    }
    else
    {
        state.SelectedIndex = 0;
    }
}

void MenuManager::CloseAllMenus(int slot)
{
    if (slot < 0 || slot >= 64)
        return;

    auto& state = _states[slot];
    state.Reset();
    MessageSystem::Instance().ClearCenterHtml(slot);
}

bool MenuManager::HasActiveMenu(int slot) const
{
    if (slot < 0 || slot >= 64)
        return false;

    return _states[slot].HasMenu();
}

void MenuManager::OnGameFrame()
{
    for (int slot = 0; slot < 64; ++slot)
    {
        auto& state = _states[slot];
        if (!state.HasMenu())
            continue;

        uint64_t buttons = EntitySystem::Instance().GetPlayerButtons(slot);
        auto prev = state.PrevButtons;
        state.PrevButtons = buttons;

        HandleInput(slot, buttons, prev);
        RenderMenu(slot);
    }
}

void MenuManager::HandleInput(int slot, uint64_t buttons, uint64_t prevButtons)
{
    auto& state = _states[slot];
    auto* menu = state.GetCurrentMenu();
    if (!menu)
        return;

    uint64_t pressed = buttons & ~prevButtons;
    if (pressed == 0)
        return;

    auto now = GetCurrentTimeMs();
    if (now - state.LastInputTime < InputDebounceMs)
        return;

    int itemCount = static_cast<int>(menu->Items.size());
    if (itemCount == 0)
        return;

    bool isPaginated = itemCount > ItemsPerPage;
    bool inputHandled = true;

    if (pressed & IN_FORWARD)
        StepCursor(menu->Items, state.SelectedIndex, -1);
    else if (pressed & IN_BACK)
        StepCursor(menu->Items, state.SelectedIndex, +1);
    else if (isPaginated && (pressed & IN_MOVELEFT))
        JumpPage(menu->Items, state.SelectedIndex, -1);
    else if (isPaginated && (pressed & IN_MOVERIGHT))
        JumpPage(menu->Items, state.SelectedIndex, +1);
    else if (pressed & IN_USE)
    {
        const auto& item = menu->Items[state.SelectedIndex];
        if (item.Enabled && item.OnSelect)
            item.OnSelect(slot);
    }
    else if (pressed & IN_RELOAD)
        CloseMenu(slot);
    else
        inputHandled = false;

    if (inputHandled)
        state.LastInputTime = now;
}

void MenuManager::RenderMenu(int slot)
{
    auto& state = _states[slot];
    auto* menu = state.GetCurrentMenu();
    if (!menu)
        return;

    bool isSubmenu = state.MenuStack.size() > 1;
    auto html = RenderMenuHtml(menu, state.SelectedIndex, isSubmenu);
    MessageSystem::Instance().SendCenterHtml(slot, html);
}

void MenuManager::OnPlayerDisconnect(int slot)
{
    if (slot < 0 || slot >= 64)
        return;

    _states[slot].Reset();
}

}  // namespace CS2Kit::Menu
