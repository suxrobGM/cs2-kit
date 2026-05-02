#pragma once

/**
 * @file Options.hpp
 * @brief Aggregate header pulling in every concrete @ref CS2Kit::Menu::MenuOption type.
 *
 * Most consumers should include @ref CS2Kit/Menu/MenuBuilder.hpp instead, which already
 * brings these in. Pull this header directly only when constructing options manually
 * (e.g. for `MenuBuilder::AddOption`) or when a custom subclass needs the base.
 */

#include <CS2Kit/Menu/MenuOption.hpp>
#include <CS2Kit/Menu/Options/ButtonOption.hpp>
#include <CS2Kit/Menu/Options/ChoiceOption.hpp>
#include <CS2Kit/Menu/Options/InputOption.hpp>
#include <CS2Kit/Menu/Options/ProgressBarOption.hpp>
#include <CS2Kit/Menu/Options/SelectorOption.hpp>
#include <CS2Kit/Menu/Options/SliderOption.hpp>
#include <CS2Kit/Menu/Options/SubmenuOption.hpp>
#include <CS2Kit/Menu/Options/TextOption.hpp>
#include <CS2Kit/Menu/Options/ToggleOption.hpp>
