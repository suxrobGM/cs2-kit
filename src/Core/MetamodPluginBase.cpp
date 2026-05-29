#include <CS2Kit/CS2Kit.hpp>
#include <CS2Kit/Core/MetamodPluginBase.hpp>
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Players/Player.hpp>
#include <CS2Kit/Players/PlayerManager.hpp>
#include <CS2Kit/Sdk/GameInterfaces.hpp>
#include <CS2Kit/Utils/Log.hpp>
#include <cstdio>
#include <cstring>
#include <string>

// extern decls for the SourceHook globals the consumer's PLUGIN_EXPOSE defines; lets the base
// own the standard hooks and call PLUGIN_SAVEVARS in Load().
PLUGIN_GLOBALVARS();

namespace CS2Kit::Core
{

using namespace CS2Kit::Players;
using namespace CS2Kit::Sdk;
using namespace CS2Kit::Utils;

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK6_void(IServerGameClients, OnClientConnected, SH_NOATTRIB, 0, CPlayerSlot, const char*, uint64, const char*,
                   const char*, bool);
SH_DECL_HOOK5_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, CPlayerSlot, ENetworkDisconnectionReason,
                   const char*, uint64, const char*);
SH_DECL_HOOK3_void(ICvar, DispatchConCommand, SH_NOATTRIB, 0, ConCommandRef, const CCommandContext&, const CCommand&);

MetamodPluginBase::MetamodPluginBase() = default;
MetamodPluginBase::~MetamodPluginBase() = default;

bool MetamodPluginBase::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();
    _lateLoad = late;

    // Fresh service container per load; wire the accessor before Initialize so kit subsystems
    // (and the plugin's OnLoad) can reach each other via Kit(). Destroyed in Unload — this is
    // what makes meta reload start from clean state.
    _services = std::make_unique<Services>();
    SetActiveServices(_services.get());

    CS2Kit::InitParams params;
    params.LogPrefix = Info().LogTag;
    if (!CS2Kit::Initialize(ismm, error, maxlen, *_services, params))
    {
        SetActiveServices(nullptr);
        _services.reset();
        return false;
    }

    RegisterStandardHooks();
    OnRegisterHooks();

    if (!OnLoad(late))
    {
        snprintf(error, maxlen, "Plugin initialization failed");
        RunDeferred();
        CS2Kit::Shutdown(*_services);
        OnDestroyInstances();
        SetActiveServices(nullptr);
        _services.reset();
        return false;
    }

    Log::Info("Loaded successfully{}.", late ? " (late)" : "");
    return true;
}

bool MetamodPluginBase::Unload(char* error, size_t maxlen)
{
    // Order matters: deferred teardown (hook removal, DB close, timer cancel) runs while every
    // instance is still alive; only then do we tear the instances down (plugin first, then kit),
    // and null the accessor before the kit services are destroyed so nothing dereferences Kit().
    OnUnload();
    RunDeferred();
    CS2Kit::Shutdown(*_services);
    OnDestroyInstances();
    SetActiveServices(nullptr);
    _services.reset();
    return true;
}

void MetamodPluginBase::Defer(std::function<void()> cleanup)
{
    _deferred.push_back(std::move(cleanup));
}

void MetamodPluginBase::RunDeferred()
{
    for (auto it = _deferred.rbegin(); it != _deferred.rend(); ++it)
    {
        if (*it)
        {
            (*it)();
        }
    }
    _deferred.clear();
}

void MetamodPluginBase::RegisterStandardHooks()
{
    auto& gi = CS2Kit::Core::Kit().Interfaces;

    SH_ADD_HOOK(IServerGameDLL, GameFrame, gi.ServerGameDLL, SH_MEMBER(this, &MetamodPluginBase::Hook_GameFrame), true);
    SH_ADD_HOOK(IServerGameClients, OnClientConnected, gi.ServerGameClients,
                SH_MEMBER(this, &MetamodPluginBase::Hook_OnClientConnected), false);
    SH_ADD_HOOK(IServerGameClients, ClientDisconnect, gi.ServerGameClients,
                SH_MEMBER(this, &MetamodPluginBase::Hook_ClientDisconnect), true);
    SH_ADD_HOOK(ICvar, DispatchConCommand, gi.CVar, SH_MEMBER(this, &MetamodPluginBase::Hook_DispatchConCommand),
                false);

    Defer([this] {
        auto& g = CS2Kit::Core::Kit().Interfaces;
        SH_REMOVE_HOOK(IServerGameDLL, GameFrame, g.ServerGameDLL, SH_MEMBER(this, &MetamodPluginBase::Hook_GameFrame),
                       true);
        SH_REMOVE_HOOK(IServerGameClients, OnClientConnected, g.ServerGameClients,
                       SH_MEMBER(this, &MetamodPluginBase::Hook_OnClientConnected), false);
        SH_REMOVE_HOOK(IServerGameClients, ClientDisconnect, g.ServerGameClients,
                       SH_MEMBER(this, &MetamodPluginBase::Hook_ClientDisconnect), true);
        SH_REMOVE_HOOK(ICvar, DispatchConCommand, g.CVar, SH_MEMBER(this, &MetamodPluginBase::Hook_DispatchConCommand),
                       false);
    });

    Log::Info("Hooks registered.");
}

void MetamodPluginBase::Hook_GameFrame(bool simulating, bool firstTick, bool lastTick)
{
    CS2Kit::OnGameFrame(*_services);
}

void MetamodPluginBase::Hook_OnClientConnected(CPlayerSlot slot, const char* name, uint64 xuid, const char* networkId,
                                               const char* address, bool fakePlayer)
{
    int slotIdx = slot.Get();
    int64_t steamId = static_cast<int64_t>(xuid);
    Player* player = CS2Kit::Core::Kit().Players.AddPlayer(slotIdx, steamId, name ? name : "", address ? address : "");
    OnPlayerConnect(player);
}

void MetamodPluginBase::Hook_ClientDisconnect(CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* name,
                                              uint64 xuid, const char* networkId)
{
    int slotIdx = slot.Get();
    OnPlayerDisconnect(CS2Kit::Core::Kit().Players.GetPlayerBySlot(slotIdx));
    CS2Kit::OnPlayerDisconnect(*_services, slotIdx);
    CS2Kit::Core::Kit().Players.RemovePlayer(slotIdx);
}

void MetamodPluginBase::Hook_DispatchConCommand(ConCommandRef cmd, const CCommandContext& ctx, const CCommand& args)
{
    const char* cmdName = cmd.GetName();
    if (!cmdName)
        return;

    bool isSay = (strcmp(cmdName, "say") == 0);
    bool isSayTeam = (strcmp(cmdName, "say_team") == 0);
    if (!isSay && !isSayTeam)
        return;

    if (args.ArgC() < 2)
        return;

    std::string message = args.Arg(1);
    if (message.size() >= 2 && message.front() == '"' && message.back() == '"')
        message = message.substr(1, message.size() - 2);
    if (message.empty())
        return;

    int slotIdx = ctx.GetPlayerSlot().Get();
    if (slotIdx < 0 || slotIdx >= 64)
        return;

    Player* player = CS2Kit::Core::Kit().Players.GetPlayerBySlot(slotIdx);
    if (!player)
        return;

    if (OnPlayerChat(player, message, isSayTeam))
        RETURN_META(MRES_SUPERCEDE);
}

const char* MetamodPluginBase::GetAuthor()
{
    return Info().Author;
}
const char* MetamodPluginBase::GetName()
{
    return Info().Name;
}
const char* MetamodPluginBase::GetDescription()
{
    return Info().Description;
}
const char* MetamodPluginBase::GetURL()
{
    return Info().Url;
}
const char* MetamodPluginBase::GetLicense()
{
    return Info().License;
}
const char* MetamodPluginBase::GetVersion()
{
    return Info().Version;
}
const char* MetamodPluginBase::GetDate()
{
    return Info().Date;
}
const char* MetamodPluginBase::GetLogTag()
{
    return Info().LogTag;
}

void* MetamodPluginBase::OnMetamodQuery(const char* iface, int* ret)
{
    if (ret)
    {
        *ret = META_IFACE_FAILED;
    }

    return nullptr;
}

}  // namespace CS2Kit::Core
