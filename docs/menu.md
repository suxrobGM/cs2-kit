# Menu System {#menus_guide}

[TOC]

> **Work in Progress** — The menu API may change.

## Overview

The menu system (`CS2Kit::Menu`) provides WASD-navigated center-HTML menus for CS2. Players interact using:

| Key | Action |
|-----|--------|
| **W** | Navigate up |
| **S** | Navigate down |
| **E** | Select item |
| **A** | Previous page (paginated menus only) |
| **D** | Next page (paginated menus only) |
| **R** | Close menu (root) / go back to parent (submenu) |

The `[R]` hint in the footer automatically renders as **Close** on a root menu and **Back** on a submenu.

## Building Menus

Use `MenuBuilder` to construct menus with a fluent API:

```cpp
#include <Menu/MenuBuilder.hpp>
#include <Menu/MenuManager.hpp>

using namespace CS2Kit::Menu;

auto menu = MenuBuilder("Admin Panel")
    .AddItem("Kick Player", [](int slot)
    {
        // Handle kick action
    })
    .AddItem("Ban Player", [](int slot)
    {
        // Handle ban action
    })
    .AddItem("Disabled Option", [](int slot) {}, false)  // disabled item
    .OnClose([](int slot)
    {
        // Cleanup when menu closes
    })
    .Build();

MenuManager::Instance().OpenMenu(playerSlot, menu);
```

## Submenus

Use `AddSubmenu` to create nested menus. The factory function is called when the item is selected:

```cpp
auto mainMenu = MenuBuilder("Main Menu")
    .AddSubmenu("Settings", [](int slot) -> std::shared_ptr<Menu>
    {
        return MenuBuilder("Settings")
            .AddItem("Option A", [](int slot) { /* ... */ })
            .AddItem("Option B", [](int slot) { /* ... */ })
            .Build();
    })
    .Build();
```

## MenuBuilder API

| Method | Description |
|--------|-------------|
| `MenuBuilder(title)` | Constructor — sets menu title |
| `.AddItem(title, handler)` | Add a selectable menu item |
| `.AddItem(title, handler, enabled)` | Add an item with explicit enabled state |
| `.AddSubmenu(title, factory, enabled)` | Add a submenu with lazy construction |
| `.OnClose(callback)` | Called when the menu is closed |
| `.WithHeader(fn)` | Custom HTML header (returns string) |
| `.WithFooter(fn)` | Custom HTML footer (returns string) |
| `.Build()` | Returns `shared_ptr<Menu>` |

## Pagination

Menus with more than `CS2Kit::Menu::ItemsPerPage` items (5 by default) automatically paginate. The current page indicator (e.g. `(2/3)`) appears next to the title, and the footer shows the `[A/D] Page` hint only when more than one page exists. No builder change is required — pagination is purely a render-time concern.

- **W / S** — move the cursor one row at a time across all items, wrapping at both ends. The visible page follows the cursor.
- **A / D** — jump a whole page back / forward, wrapping at both ends. The cursor preserves its in-page offset where possible (row 3 of page 1 → row 3 of page 2, or the last row if the new page is shorter).

Disabled items are skipped during both per-row and per-page navigation, so the cursor will never land on a non-selectable row.

## Custom Layout

Customize the header and footer HTML rendered above/below menu items:

```cpp
auto menu = MenuBuilder("Custom Menu")
    .WithHeader([]() { return "<b>Server Admin</b><br><i>v1.0</i>"; })
    .WithFooter([]() { return "<font color='gray'>Use WASD to navigate</font>"; })
    .AddItem("Item 1", [](int slot) {})
    .Build();
```

## IMenuIO Interface

Your plugin must implement `IMenuIO` to bridge the menu system with the engine:

```cpp
class IMenuIO
{
public:
    virtual ~IMenuIO() = default;
    virtual uint64_t GetPlayerButtons(int slot) = 0;
    virtual void SendCenterHtml(int slot, const std::string& html) = 0;
    virtual void ClearCenterHtml(int slot) = 0;
};
```

Register it during initialization:

```cpp
CS2Kit::Menu::MenuManager::Instance().SetIO(&myMenuIO);
```

## Menu Lifecycle

`MenuManager` maintains a per-player menu stack:

1. `OpenMenu()` pushes a menu onto the stack
2. Player navigates with W/S, selects with E
3. Submenus push onto the stack; R pops back
4. `CloseMenu()` clears the entire stack
5. Input is debounced (200ms) to prevent accidental double-presses

Call `MenuManager::OnGameFrame()` every tick and `MenuManager::OnPlayerDisconnect(slot)` on disconnect.
