# Menu System {#menus_guide}

[TOC]

> **Work in Progress** — The menu API may change.

## Overview

The menu system (`CS2Kit::Menu`) provides WASD-navigated center-HTML menus for CS2. Each row is a typed @ref CS2Kit::Menu::MenuOption — buttons, toggles, choice cycles, sliders, progress bars, free-text inputs, and submenu links — built fluently with @ref CS2Kit::Menu::MenuBuilder.

Players interact using:

| Key | Action |
| --- | --- |
| **W** | Navigate up |
| **S** | Navigate down |
| **E** | Activate the highlighted row (button click, toggle flip, choice commit, input prompt, …) |
| **A** | If the highlighted row is a value option (Toggle / Choice / Selector / Slider): adjust value left. Otherwise: previous page (paginated menus only). |
| **D** | Same as **A**, in the opposite direction. |
| **R** | Close menu (root) / go back to parent (submenu). Cancels an active chat-input capture. |

The `[R]` hint in the footer renders as **Close** on a root menu and **Back** on a submenu.

## Building Menus

Use @ref CS2Kit::Menu::MenuBuilder to construct menus with a fluent API. Pull `MenuBuilder.hpp` — it includes every concrete option type via the aggregate `Options.hpp`:

```cpp
#include <CS2Kit/Menu/MenuBuilder.hpp>
#include <CS2Kit/Menu/MenuManager.hpp>

using namespace CS2Kit::Menu;

auto menu = MenuBuilder("Admin Panel")
    .AddButton("Kick Player", [](int slot) { /* … */ })
    .AddButton("Ban Player",  [](int slot) { /* … */ })
    .AddButton("Disabled",    [](int slot) {}, /*enabled=*/false)
    .OnClose([](int slot) { /* cleanup */ })
    .Build();

MenuManager::Instance().OpenMenu(playerSlot, menu);
```

## Option Types

Every row is a @ref CS2Kit::Menu::MenuOption subclass. The builder methods construct the right type for you; only reach for `AddOption(std::shared_ptr<MenuOption>)` if you need a custom subclass.

### Text — non-selectable label

`AddText(label)` — heading or divider. Rendered in muted color, skipped by W/S.

### Button — plain action

`AddButton(label, onActivate, enabled = true)` — fires the callback on E. The dynamic-label variant `AddDynamicButton(getLabel, onActivate, enabled = true)` recomputes the label every frame, useful when the row reflects live state but isn't a toggle.

```cpp
.AddButton("Slay", [admin, target](int) { Actions::DoSlay(admin, target); }, hasSlayPerm)
```

### Toggle — boolean state

`AddToggle(title, onLabel, offLabel, getState, onToggle, enabled = true)` — renders `"<title>: <onLabel|offLabel>"`. Both **E** and **A/D** flip. State lives wherever the caller keeps it (engine field, an `EffectManager`, a config struct); pass getter and toggle callbacks.

```cpp
.AddToggle("Beacon", "ON", "OFF",
    [slot](int) { return EffectManager::Instance().IsActive(slot, EffectId::Beacon); },
    [slot](int) { Effects::ToggleBeacon(slot); })
```

### Choice — string-labeled cycle

`AddChoice<T>(title, choices, getIndex, setIndex, onCommit = nullptr, enabled = true)` — A/D walks the list (wrapping), E commits the current value. Each choice is `{label, value}`; `T` is whatever you want to pass to `onCommit`. State is *index-based* and external — the caller decides where the index lives (a captured `std::shared_ptr<int>` is fine for ephemeral menu state).

```cpp
auto idx = std::make_shared<int>(0);
builder.AddChoice<int>(
    "HP", {{"1 HP", 1}, {"100 HP", 100}, {"999 HP", 999}},
    [idx](int) { return *idx; },
    [idx](int, int newIdx) { *idx = newIdx; },
    [admin, target](int slot, const int& hp) {
        Actions::DoSetHealth(admin, target, hp);
        MenuManager::Instance().CloseAllMenus(slot);
    });
```

### Selector — formatted cycle for non-string values

`AddSelector<T>(title, values, formatter, getIndex, setIndex, onCommit = nullptr, enabled = true)` — like Choice, but you supply a `std::function<std::string(const T&)>` to derive the label. Use it when the value type doesn't carry its own pretty name (e.g. seconds → `"5m"`, an enum → a translated label).

### Slider — numeric range

`AddSlider(title, min, max, step, getValue, setValue, enabled = true)` — A/D adjusts in `step` units, clamped to `[min, max]`. Renders `"<title>: [▮▮▮▯▯▯▯▯▯▯] 30/100"`. E does nothing by default.

```cpp
.AddSlider("Speed", /*min=*/100, /*max=*/500, /*step=*/50,
    [slot](int)         { return GetSpeed(slot); },
    [slot](int, int v)  { SetSpeed(slot, v); })
```

### ProgressBar — read-only

`AddProgressBar(title, getValue, max)` — non-selectable. Renders the same bar shape as Slider but is skipped by the cursor.

### Input — free-text via chat

`AddInput(title, prompt, get, set, maxLength = 64, enabled = true)` — pressing E pauses the menu, shows the prompt overlay, and routes the player's next chat line into the validator. Return `false` from `set` to re-prompt for invalid input; `true` accepts and resumes the menu. **R** during capture cancels.

```cpp
.AddInput("Custom duration", "Enter duration (e.g. 30s, 5m, 2h, 7d)",
    [](int) { return std::string{}; },
    [target](int slot, std::string_view text) -> bool {
        int seconds = ParseDuration(text);
        if (seconds < 0) return false;          // re-prompt
        IssueBan(slot, target, seconds);
        return true;
    },
    /*maxLength=*/32)
```

This relies on @ref CS2Kit::Sdk::ChatInputCapture — the plugin must call `ChatInputCapture::Instance().TryConsume(slot, text)` from its chat-message hook before its own command parsing, suppressing the chat broadcast when the call returns `true`. See the [SDK guide](@ref sdk_guide) for the integration snippet.

### Submenu — push a built submenu

`AddSubmenu(label, factory, enabled = true)` — the factory is invoked lazily on E, and the returned menu is pushed onto the player's stack. R pops back to the parent.

```cpp
.AddSubmenu("Settings", [](int slot) {
    return MenuBuilder("Settings")
        .AddButton("Option A", [](int) { /* … */ })
        .Build();
})
```

### AddOption — escape hatch

`AddOption(std::shared_ptr<MenuOption>)` lets you append a custom subclass. Override `GetLabel(slot)`, `OnActivate(slot)`, and optionally `OnHorizontal(slot, direction)` (return `true` to consume A/D, `false` to fall through to page-jump).

## Pagination

Menus with more than `CS2Kit::Menu::ItemsPerPage` items (5 by default) automatically paginate. The page indicator (e.g. `(2/3)`) appears next to the title. The footer shows the `[A/D] Page` hint only when more than one page exists.

A/D is **item-aware**: when the highlighted row is a value option (Toggle / Choice / Selector / Slider), A/D adjusts its value and pagination is *not* triggered. Highlight a plain Button or Submenu row to page through.

Disabled rows and non-selectable rows (Text, ProgressBar) are skipped during W/S navigation, so the cursor never lands on a row you can't act on.

## Custom Layout

Override the default header / footer with your own HTML:

```cpp
auto menu = MenuBuilder("Custom Menu")
    .WithHeader([] { return "<b>Server Admin</b><br><i>v1.0</i>"; })
    .WithFooter([] { return "<font color='gray'>WASD to navigate</font>"; })
    .AddButton("Item 1", [](int) {})
    .Build();
```

## Lifecycle

@ref CS2Kit::Menu::MenuManager keeps a per-player menu stack and runs entirely on the game thread:

1. `OpenMenu(slot, menu)` pushes the menu and rendering begins.
2. `OnGameFrame()` reads `IN_FORWARD/IN_BACK/IN_USE/IN_RELOAD/IN_MOVELEFT/IN_MOVERIGHT` each tick and dispatches via the @ref CS2Kit::Menu::MenuOption virtuals.
3. Submenus push onto the stack; **R** pops back. `CloseAllMenus(slot)` clears the entire stack.
4. Input is debounced (200ms) to prevent accidental double-presses.
5. While a chat-input capture is active for the slot, only **R** is honored — every other key is ignored so the cursor doesn't drift while the player types.

The plugin must call `CS2Kit::OnGameFrame()` every tick (which drives `MenuManager::OnGameFrame()` internally) and `CS2Kit::OnPlayerDisconnect(slot)` on disconnect.

## Header Layout

```text
include/CS2Kit/Menu/
├── Menu.hpp                  Menu / MenuLayout / PlayerMenuState
├── MenuOption.hpp            Polymorphic base only
├── MenuBuilder.hpp           Fluent builder (pulls in Options.hpp)
├── MenuManager.hpp           Stack + tick driver + disconnect hook
├── Options.hpp               Aggregate include for every concrete option
└── Options/                  One file per concrete option subclass
    ├── ButtonOption.hpp
    ├── ChoiceOption.hpp
    ├── InputOption.hpp
    ├── ProgressBarOption.hpp
    ├── SelectorOption.hpp
    ├── SliderOption.hpp
    ├── SubmenuOption.hpp
    ├── TextOption.hpp
    ├── ToggleOption.hpp
    └── Bar.hpp               Shared unicode-bar rendering helper
```

Most consumers only need `MenuBuilder.hpp` and `MenuManager.hpp`. Pull `Options.hpp` (or an individual `Options/*.hpp`) only when you need to construct an option manually for `AddOption`, or when defining a custom `MenuOption` subclass.
