#include "ConVarService.hpp"

#include "../Utils/Log.hpp"
#include "GameInterfaces.hpp"

#include <icvar.h>
#include <tier1/convar.h>

namespace
{

void GlobalConVarChangeCallback(ConVarRefAbstract* ref, CSplitScreenSlot /*slot*/, const char* newValue,
                                const char* oldValue, void* /*unk*/)
{
    if (!ref)
        return;

    const char* name = ref->GetName();
    CS2Kit::Sdk::ConVarService::Instance().DispatchChange(name, oldValue, newValue);
}

}  // namespace

namespace CS2Kit::Sdk
{
using namespace CS2Kit::Utils;

bool ConVarService::Initialize()
{
    if (!GameInterfaces::Instance().CVar)
    {
        Log::Error("ConVarService: ICvar not available.");
        return false;
    }

    Log::Info("ConVar service initialized.");
    return true;
}

std::optional<int> ConVarService::GetInt(const char* name) const
{
    ConVarRefAbstract ref(name);
    if (!ref.IsValidRef() || !ref.IsConVarDataAvailable())
        return std::nullopt;

    return ref.GetInt();
}

std::optional<float> ConVarService::GetFloat(const char* name) const
{
    ConVarRefAbstract ref(name);
    if (!ref.IsValidRef() || !ref.IsConVarDataAvailable())
        return std::nullopt;

    return ref.GetFloat();
}

std::optional<std::string> ConVarService::GetString(const char* name) const
{
    ConVarRefAbstract ref(name);
    if (!ref.IsValidRef() || !ref.IsConVarDataAvailable())
        return std::nullopt;

    CUtlString str = ref.GetString();
    return std::string(str.Get());
}

std::optional<bool> ConVarService::GetBool(const char* name) const
{
    ConVarRefAbstract ref(name);
    if (!ref.IsValidRef() || !ref.IsConVarDataAvailable())
        return std::nullopt;

    return ref.GetBool();
}

bool ConVarService::Exists(const char* name) const
{
    ConVarRefAbstract ref(name);
    return ref.IsValidRef() && ref.IsConVarDataAvailable();
}

bool ConVarService::SetInt(const char* name, int value)
{
    ConVarRefAbstract ref(name);
    if (!ref.IsValidRef() || !ref.IsConVarDataAvailable())
        return false;

    ref.SetInt(value);
    return true;
}

bool ConVarService::SetFloat(const char* name, float value)
{
    ConVarRefAbstract ref(name);
    if (!ref.IsValidRef() || !ref.IsConVarDataAvailable())
        return false;

    ref.SetFloat(value);
    return true;
}

bool ConVarService::SetString(const char* name, const char* value)
{
    ConVarRefAbstract ref(name);
    if (!ref.IsValidRef() || !ref.IsConVarDataAvailable())
        return false;

    ref.SetString(CUtlString(value));
    return true;
}

void ConVarService::ExecuteServerCommand(const char* command)
{
    auto* engine = GameInterfaces::Instance().Engine;
    if (!engine)
    {
        Log::Warn("ConVarService::ExecuteServerCommand: IVEngineServer2 not available.");
        return;
    }

    engine->ServerCommand(command);
}

uint64_t ConVarService::OnChange(ChangeCallback callback)
{
    if (!_globalCallbackInstalled)
    {
        auto* cvar = GameInterfaces::Instance().CVar;
        if (cvar)
        {
            cvar->InstallGlobalChangeCallback(&GlobalConVarChangeCallback);
            _globalCallbackInstalled = true;
        }
    }

    uint64_t id = _nextCallbackId++;
    _changeCallbacks[id] = std::move(callback);
    return id;
}

void ConVarService::RemoveChangeListener(uint64_t id)
{
    _changeCallbacks.erase(id);
}

void ConVarService::DispatchChange(const char* name, const char* oldValue, const char* newValue)
{
    for (auto& [id, callback] : _changeCallbacks)
    {
        callback(name, oldValue, newValue);
    }
}

}  // namespace CS2Kit::Sdk
