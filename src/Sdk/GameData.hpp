#pragma once

#include "../Core/Singleton.hpp"

#include <string>
#include <unordered_map>

namespace CS2Kit::Sdk
{

/**
 * Centralized gamedata manager for platform-specific signatures and offsets.
 * Loads byte-pattern signatures and named integer offsets from a JSON file,
 * then resolves signatures at runtime via SigScanner.
 */
class GameData : public Core::Singleton<GameData>
{
public:
    explicit GameData(Token) {}

    bool Load(const std::string& path);
    int GetOffset(const std::string& name) const;
    void* FindSignature(const std::string& name) const;
    void* ResolveSignature(const std::string& name) const;

private:
    struct SignatureEntry
    {
        std::string Library;
        std::string Pattern;
        int Offset = 0;
    };

    std::unordered_map<std::string, int> _offsets;
    std::unordered_map<std::string, SignatureEntry> _signatures;
};

}  // namespace CS2Kit::Sdk
