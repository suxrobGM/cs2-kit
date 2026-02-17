#pragma once

#include <cstdint>
#include <string>

namespace CS2Kit::Menu
{

/**
 * Menu input/output interface.
 * MenuManager uses this to read player button states and send HTML to the center HUD.
 * Consumers implement this using their SDK wrappers (EntitySystem, MessageSystem, etc.).
 */
class IMenuIO
{
public:
    virtual ~IMenuIO() = default;

    virtual uint64_t GetPlayerButtons(int slot) = 0;
    virtual void SendCenterHtml(int slot, const std::string& html) = 0;
    virtual void ClearCenterHtml(int slot) = 0;
};

}  // namespace CS2Kit::Menu
