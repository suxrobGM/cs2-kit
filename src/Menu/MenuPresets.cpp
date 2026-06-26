#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Menu/MenuBuilder.hpp>
#include <CS2Kit/Menu/MenuPresets.hpp>
#include <CS2Kit/Players/PlayerManager.hpp>
#include <CS2Kit/Utils/StringUtils.hpp>
#include <string_view>
#include <utility>

using CS2Kit::Core::Engine;

namespace CS2Kit::Menu
{

std::shared_ptr<Menu> BuildPlayerPicker(int viewerSlot, const std::string& title,
                                        std::function<void(int viewerSlot, int targetSlot)> onPick,
                                        const std::string& emptyLabel, std::function<bool(int targetSlot)> isEnabled)
{
    MenuBuilder builder(title);

    auto players = Engine().Players.GetAllPlayers();
    for (auto* p : players)
    {
        int targetSlot = p->GetSlot();
        bool enabled = isEnabled ? isEnabled(targetSlot) : true;
        builder.AddButton(
            p->GetName(),
            [viewerSlot, targetSlot, onPick](int /*slot*/) {
                if (onPick)
                    onPick(viewerSlot, targetSlot);
            },
            enabled);
    }

    if (players.empty() && !emptyLabel.empty())
        builder.AddButton(emptyLabel, [](int) {}, false);

    return builder.Build();
}

std::shared_ptr<Menu> BuildDurationPicker(int viewerSlot, const std::string& title,
                                          const std::vector<std::pair<std::string, int>>& presets,
                                          std::function<void(int viewerSlot, int seconds)> onPick,
                                          const std::string& customLabel, const std::string& customPrompt,
                                          int maxInputLen)
{
    MenuBuilder builder(title);

    for (const auto& [label, seconds] : presets)
    {
        builder.AddButton(label, [viewerSlot, secs = seconds, onPick](int /*slot*/) {
            if (onPick)
                onPick(viewerSlot, secs);
        });
    }

    if (!customLabel.empty())
    {
        builder.AddInput(
            customLabel, customPrompt, [](int) { return std::string{}; },
            [viewerSlot, onPick](int /*slot*/, std::string_view text) -> bool {
                int seconds = Utils::ParseDuration(text);
                if (seconds < 0)
                    return false;  // re-prompt
                if (onPick)
                    onPick(viewerSlot, seconds);
                return true;
            },
            maxInputLen);
    }

    return builder.Build();
}

}  // namespace CS2Kit::Menu
