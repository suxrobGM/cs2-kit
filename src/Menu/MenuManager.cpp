#include <CS2Kit/Menu/MenuManager.hpp>

#include <CS2Kit/Utils/Log.hpp>

#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>

#include "Menu/MenuRenderer.hpp"

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

    bool inputHandled = false;

    if (pressed & IN_FORWARD)
    {
        state.SelectedIndex = (state.SelectedIndex - 1 + itemCount) % itemCount;

        int attempts = itemCount;
        while (!menu->Items[state.SelectedIndex].Enabled && --attempts > 0)
            state.SelectedIndex = (state.SelectedIndex - 1 + itemCount) % itemCount;

        inputHandled = true;
    }
    else if (pressed & IN_BACK)
    {
        state.SelectedIndex = (state.SelectedIndex + 1) % itemCount;

        int attempts = itemCount;
        while (!menu->Items[state.SelectedIndex].Enabled && --attempts > 0)
            state.SelectedIndex = (state.SelectedIndex + 1) % itemCount;

        inputHandled = true;
    }
    else if (pressed & IN_USE)
    {
        if (state.SelectedIndex >= 0 && state.SelectedIndex < itemCount)
        {
            const auto& item = menu->Items[state.SelectedIndex];
            if (item.Enabled && item.OnSelect)
                item.OnSelect(slot);
        }
        inputHandled = true;
    }
    else if (pressed & IN_RELOAD)
    {
        CloseMenu(slot);
        inputHandled = true;
    }

    if (inputHandled)
    {
        state.LastInputTime = now;
    }
}

void MenuManager::RenderMenu(int slot)
{
    auto& state = _states[slot];
    auto* menu = state.GetCurrentMenu();
    if (!menu)
        return;

    auto html = RenderMenuHtml(menu, state.SelectedIndex);
    MessageSystem::Instance().SendCenterHtml(slot, html);
}

void MenuManager::OnPlayerDisconnect(int slot)
{
    if (slot < 0 || slot >= 64)
        return;

    _states[slot].Reset();
}

}  // namespace CS2Kit::Menu
