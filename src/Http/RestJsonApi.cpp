#include <CS2Kit/Http/RestJsonApi.hpp>
#include <CS2Kit/Utils/Json.hpp>
#include <CS2Kit/Utils/StringUtils.hpp>
#include <utility>

namespace CS2Kit::Http
{

using Utils::Json;
using Utils::StringUtils;

std::optional<PreparedRequest> BuildJsonPost(const JsonPostSpec& spec, const std::map<std::string, std::string>& tokens)
{
    if (spec.Url.empty())
        return std::nullopt;

    nlohmann::json body = spec.BodyTemplate.is_null() ? nlohmann::json::object() : spec.BodyTemplate;
    Json::SubstituteTokens(body, tokens);

    std::vector<std::string> headers = {"Content-Type: application/json"};
    if (!spec.ApiKey.empty())
    {
        const std::string value = spec.AuthScheme.empty() ? spec.ApiKey : spec.AuthScheme + " " + spec.ApiKey;
        headers.push_back(spec.AuthHeader + ": " + value);
    }

    return PreparedRequest{
        .Url = spec.Url,
        .Body = body.dump(),
        .Headers = std::move(headers),
        .TimeoutMs = spec.TimeoutMs,
    };
}

bool IsSuccess(const HttpResult& result)
{
    return result.Ok && result.StatusCode >= 200 && result.StatusCode < 300;
}

std::string ExtractField(const HttpResult& result, std::string_view dotPath, const std::string& valueTemplate)
{
    if (dotPath.empty())
        return {};

    const auto json = nlohmann::json::parse(result.Body, nullptr, /*allow_exceptions=*/false);
    if (!json.is_structured())
        return {};

    const std::string raw = Json::GetStringByPath(json, dotPath);
    if (raw.empty())
        return {};
    return valueTemplate.empty() ? raw : StringUtils::SubstituteTokens(valueTemplate, {{"value", raw}});
}

void Post(HttpClient& client, PreparedRequest request, HttpCompletion onComplete)
{
    client.Post(std::move(request.Url), std::move(request.Body), std::move(request.Headers), request.TimeoutMs,
                std::move(onComplete));
}

}  // namespace CS2Kit::Http
