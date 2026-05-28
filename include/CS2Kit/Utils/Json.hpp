#pragma once

#include <CS2Kit/Core/Paths.hpp>
#include <CS2Kit/Utils/Log.hpp>

#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace CS2Kit::Utils
{

/**
 * @brief System.Text.Json-style helpers for mapping C++ types to and from JSON.
 *
 * Works with any type that has nlohmann `to_json`/`from_json` — easiest via the
 * `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT` macro, which keys on member names
 * (they must match the JSON keys) and defaults any missing key. The `Try*` variants
 * catch and log instead of throwing; a missing key is fine, a wrong-typed value is not.
 *
 * @code
 * struct Cfg { std::string host = "localhost"; int port = 5432; };
 * NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Cfg, host, port)
 * auto cfg = Json::TryDeserializeFile<Cfg>("addons/my/config.json");
 * @endcode
 */
class Json
{
public:
    /** @brief Parse JSON text into T. Throws on malformed JSON or a type mismatch. */
    template <typename T>
    static T Deserialize(std::string_view text)
    {
        return nlohmann::json::parse(text).template get<T>();
    }

    /** @brief Serialize a value to a JSON string. @p pretty enables 2-space indentation. */
    template <typename T>
    static std::string Serialize(const T& value, bool pretty = false)
    {
        const nlohmann::json j = value;
        return pretty ? j.dump(2) : j.dump();
    }

    /** @brief Parse a JSON file into T (path resolved via ResolvePath). Returns nullopt and logs on any error. */
    template <typename T>
    static std::optional<T> TryDeserializeFile(const std::string& path)
    {
        try
        {
            auto resolved = Core::ResolvePath(path);
            std::ifstream file(resolved);
            if (!file.is_open())
            {
                Log::Error("Json: failed to open {}", resolved.string());
                return std::nullopt;
            }

            nlohmann::json j;
            file >> j;
            return j.template get<T>();
        }
        catch (const std::exception& e)
        {
            Log::Error("Json: error reading {}: {}", path, e.what());
            return std::nullopt;
        }
    }

    /** @brief Write a value to a JSON file (path resolved via ResolvePath). Returns false and logs on failure. */
    template <typename T>
    static bool SerializeToFile(const std::string& path, const T& value, bool pretty = true)
    {
        try
        {
            auto resolved = Core::ResolvePath(path);
            std::ofstream file(resolved);
            if (!file.is_open())
            {
                Log::Error("Json: failed to open {} for writing", resolved.string());
                return false;
            }

            const nlohmann::json j = value;
            file << j.dump(pretty ? 2 : -1);
            return true;
        }
        catch (const std::exception& e)
        {
            Log::Error("Json: error writing {}: {}", path, e.what());
            return false;
        }
    }
};

}  // namespace CS2Kit::Utils
