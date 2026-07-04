#pragma once

// Umbrella of curated short names for the CS2Kit public vocabulary.
//
// The library groups its API into module namespaces (Sdk, Menu, Players, ...),
// which is useful internally but forces three-segment call sites like
// `CS2Kit::Sdk::PlayerController`. This header hoists the commonly-used public
// types and free functions to the top-level `CS2Kit` namespace so consumers can
// write `CS2Kit::PlayerController` instead - in headers and sources alike,
// without a namespace-scope using-directive (which would be unsafe in a header).
//
// The fully-qualified module names keep working; the short forms are synonyms.
// Include this header wherever the short `CS2Kit::Type` spelling is used.

#include <CS2Kit/Commands/Command.hpp>
#include <CS2Kit/Commands/CommandManager.hpp>
#include <CS2Kit/Core/EffectManager.hpp>
#include <CS2Kit/Core/MetamodPluginBase.hpp>
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Database/DbResult.hpp>
#include <CS2Kit/Database/Migrator.hpp>
#include <CS2Kit/Database/PostgresDatabase.hpp>
#include <CS2Kit/Http/HttpResult.hpp>
#include <CS2Kit/Menu/Menu.hpp>
#include <CS2Kit/Menu/MenuBuilder.hpp>
#include <CS2Kit/Menu/MenuManager.hpp>
#include <CS2Kit/Menu/MenuOption.hpp>
#include <CS2Kit/Menu/MenuPresets.hpp>
#include <CS2Kit/Menu/Options/ChoiceOption.hpp>
#include <CS2Kit/Players/ActionDispatcher.hpp>
#include <CS2Kit/Players/Player.hpp>
#include <CS2Kit/Players/PlayerManager.hpp>
#include <CS2Kit/Players/TargetResolver.hpp>
#include <CS2Kit/Sdk/ChatInputCapture.hpp>
#include <CS2Kit/Sdk/ConVarService.hpp>
#include <CS2Kit/Sdk/Entity.hpp>
#include <CS2Kit/Sdk/EntityKeyValues.hpp>
#include <CS2Kit/Sdk/EntityOps.hpp>
#include <CS2Kit/Sdk/GameEventService.hpp>
#include <CS2Kit/Sdk/GlowVision.hpp>
#include <CS2Kit/Sdk/MoveType.hpp>
#include <CS2Kit/Sdk/PawnOps.hpp>
#include <CS2Kit/Sdk/PersistentCenterHtml.hpp>
#include <CS2Kit/Sdk/PlayerController.hpp>
#include <CS2Kit/Sdk/UserMessage.hpp>
#include <CS2Kit/Utils/SlotThrottle.hpp>
#include <CS2Kit/Utils/StringUtils.hpp>
#include <CS2Kit/Utils/TimeUtils.hpp>
#include <CS2Kit/Utils/Translations.hpp>

namespace CS2Kit
{

// Core
using Core::Engine;
using Core::Services;
using Core::MetamodPluginBase;
using Core::PluginInfo;
using Core::EffectManager;
using Core::EffectSpec;

// Sdk
using Sdk::PlayerController;
using Sdk::MoveType;
using Sdk::MessageSystem;
using Sdk::GlowVision;
using Sdk::EntityKeyValues;
using Sdk::PersistentCenterHtml;
using Sdk::ChatInputCapture;
using Sdk::EntitySystem;
using Sdk::EntityOpsService;
using Sdk::ConVarService;
using Sdk::GameEventService;
namespace PawnOps = Sdk::PawnOps;

// Menu  (the built-menu data model is Menu::MenuView; `Menu` is the namespace)
using Menu::MenuView;
using Menu::MenuBuilder;
using Menu::MenuOption;
using Menu::MenuManager;
using Menu::ChoiceOption;
using Menu::ConfirmDialogSpec;
using Menu::BuildConfirmDialog;
using Menu::BuildPlayerPicker;
using Menu::BuildDurationPicker;
using Menu::BuildPaletteChoices;
using Menu::AppendPlayerRows;

// Players
using Players::Player;
using Players::PlayerManager;
using Players::ActionContext;
using Players::Action;
using Players::ParamAction;
using Players::ActionDispatcher;
using Players::CanTargetFn;
using Players::ResolveSingleTarget;
using Players::SingleTargetError;
using Players::SingleTargetResult;

// Commands
using Commands::Command;
using Commands::CommandManager;
using Commands::CommandResult;

// Database
using Database::PostgresDatabase;
using Database::DbResult;
using Database::TryOr;
using Database::RunMigrations;

// Http
using Http::HttpResult;

// Utils
using Utils::TimeUtils;
using Utils::Translations;
using Utils::Tokens;
using Utils::StringUtils;
using Utils::ParseDuration;
using Utils::SlotThrottle;

}  // namespace CS2Kit
