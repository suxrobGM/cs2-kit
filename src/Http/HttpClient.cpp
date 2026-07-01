#include <CS2Kit/Http/HttpClient.hpp>
#include <chrono>
#include <cpr/cpr.h>
#include <future>
#include <utility>

namespace CS2Kit::Http
{

struct HttpClient::Impl
{
    struct Pending
    {
        std::future<HttpResult> Result;
        HttpCompletion OnComplete;
    };

    std::vector<Pending> Items;
};

HttpClient::HttpClient() : _impl(std::make_unique<Impl>()) {}

HttpClient::~HttpClient() = default;

void HttpClient::Start() {}

void HttpClient::Stop()
{
    // Block until in-flight requests finish, then drop their (unrun) completions. Joining here - on
    // the plugin Unload path, not in DllMain - is what keeps `meta reload` from leaving live worker
    // threads pointing into the unmapped DLL.
    for (auto& p : _impl->Items)
        p.Result.wait();
    _impl->Items.clear();
}

void HttpClient::Post(std::string url, std::string body, std::vector<std::string> headers, long timeoutMs,
                      HttpCompletion onComplete)
{
    // The worker touches only CPR + strings, never engine state; the callback is replayed on the
    // game thread in DispatchCompletions().
    auto task = [url = std::move(url), body = std::move(body), headers = std::move(headers),
                 timeoutMs]() -> HttpResult {
        cpr::Header header;
        for (const auto& line : headers)
        {
            const size_t colon = line.find(':');
            if (colon == std::string::npos)
                continue;
            size_t valueStart = colon + 1;
            while (valueStart < line.size() && line[valueStart] == ' ')
                ++valueStart;
            header[line.substr(0, colon)] = line.substr(valueStart);
        }

        cpr::Response response =
            cpr::Post(cpr::Url{url}, cpr::Body{body}, header, cpr::Timeout{std::chrono::milliseconds{timeoutMs}});

        HttpResult result;
        if (response.error)
        {
            result.Error = response.error.message;
        }
        else
        {
            result.Ok = true;
            result.StatusCode = static_cast<long>(response.status_code);
            result.Body = std::move(response.text);
        }
        return result;
    };

    _impl->Items.push_back({std::async(std::launch::async, std::move(task)), std::move(onComplete)});
}

void HttpClient::DispatchCompletions()
{
    auto& items = _impl->Items;

    // Collect ready completions and compact the list *before* invoking callbacks: a callback may
    // re-enter Post() and append to items.
    std::vector<Impl::Pending> ready;
    for (auto it = items.begin(); it != items.end();)
    {
        if (it->Result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            ready.push_back(std::move(*it));
            it = items.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto& p : ready)
    {
        if (p.OnComplete)
            p.OnComplete(p.Result.get());
    }
}

}  // namespace CS2Kit::Http
