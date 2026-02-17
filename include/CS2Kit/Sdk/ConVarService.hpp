#pragma once

#include <CS2Kit/Core/Singleton.hpp>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace CS2Kit::Sdk
{

/**
 * @brief Typed wrapper around ICvar for finding, reading, writing, and listening to ConVars.
 */
class ConVarService : public Core::Singleton<ConVarService>
{
public:
    explicit ConVarService(Token) {}

    bool Initialize();

    std::optional<int> GetInt(const char* name) const;
    std::optional<float> GetFloat(const char* name) const;
    std::optional<std::string> GetString(const char* name) const;
    std::optional<bool> GetBool(const char* name) const;
    bool Exists(const char* name) const;

    bool SetInt(const char* name, int value);
    bool SetFloat(const char* name, float value);
    bool SetString(const char* name, const char* value);

    void ExecuteServerCommand(const char* command);

    using ChangeCallback = std::function<void(const char* name, const char* oldValue, const char* newValue)>;
    uint64_t OnChange(ChangeCallback callback);
    void RemoveChangeListener(uint64_t id);
    void DispatchChange(const char* name, const char* oldValue, const char* newValue);

private:
    std::unordered_map<uint64_t, ChangeCallback> _changeCallbacks;
    uint64_t _nextCallbackId = 1;
    bool _globalCallbackInstalled = false;
};

}  // namespace CS2Kit::Sdk
