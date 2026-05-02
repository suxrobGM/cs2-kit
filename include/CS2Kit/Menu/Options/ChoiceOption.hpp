#pragma once

#include <CS2Kit/Menu/MenuOption.hpp>
#include <algorithm>
#include <format>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace CS2Kit::Menu
{

/**
 * Cycle a string-labeled option. A/D walks the list (wrap), E commits the current value.
 * @tparam T  value type carried with each label.
 */
template <typename T>
class ChoiceOption : public MenuOption
{
public:
    struct Choice
    {
        std::string Label;
        T Value;
    };
    using GetIndexFn = std::function<int(int)>;
    using SetIndexFn = std::function<void(int, int)>;
    using CommitFn = std::function<void(int, const T&)>;

    ChoiceOption(std::string title, std::vector<Choice> choices, GetIndexFn getIndex, SetIndexFn setIndex,
                 CommitFn onCommit = nullptr, bool enabled = true)
        : _title(std::move(title)),
          _choices(std::move(choices)),
          _getIndex(std::move(getIndex)),
          _setIndex(std::move(setIndex)),
          _onCommit(std::move(onCommit))
    {
        _enabled = enabled;
    }

    std::string GetLabel(int slot) const override
    {
        if (_choices.empty())
            return _title;
        return std::format("{}: &lt; {} &gt;", _title, _choices[ClampIndex(slot)].Label);
    }

    void OnActivate(int slot) override
    {
        if (!_enabled || _choices.empty())
            return;

        if (_onCommit)
        {
            _onCommit(slot, _choices[ClampIndex(slot)].Value);
            return;
        }

        // No explicit commit callback — E advances like D so the row stays interactive
        // for plain "pick a value" menus where there's no separate apply step.
        OnHorizontal(slot, +1);
    }

    bool OnHorizontal(int slot, int direction) override
    {
        if (!_enabled || _choices.empty() || !_setIndex)
            return false;

        int n = static_cast<int>(_choices.size());
        int idx = ClampIndex(slot);
        idx = ((idx + direction) % n + n) % n;
        _setIndex(slot, idx);
        return true;
    }

private:
    int ClampIndex(int slot) const
    {
        int n = static_cast<int>(_choices.size());
        int idx = _getIndex ? _getIndex(slot) : 0;
        return std::clamp(idx, 0, n - 1);
    }

    std::string _title;
    std::vector<Choice> _choices;
    GetIndexFn _getIndex;
    SetIndexFn _setIndex;
    CommitFn _onCommit;
};

}  // namespace CS2Kit::Menu
