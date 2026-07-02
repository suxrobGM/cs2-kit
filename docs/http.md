# HTTP {#http_guide}

[TOC]

## Overview

The HTTP module (`CS2Kit::Http`) provides:

- **HttpClient** - async POSTs on CPR's worker pool, with completions queued and replayed on the
  game thread
- **RestJsonApi** - helpers for the config-driven "call an operator-configured JSON endpoint and
  pull a field out of the response" shape

`HttpClient` is a kit service: reach it via `Engine().Http`. Completions dispatch from the
`CS2Kit::OnGameFrame()` pump and `CS2Kit::Shutdown()` drains in-flight requests, so there is no
per-plugin dispatch timer to wire up.

## Posting

```cpp
#include <CS2Kit/Core/Services.hpp>
#include <CS2Kit/Http/HttpClient.hpp>

Engine().Http.Post(url, body, {"Content-Type: application/json"}, /*timeoutMs=*/8000,
                   [](const CS2Kit::Http::HttpResult& result) {
                       // Runs on the game thread - safe to touch engine state here.
                       if (!result.Ok)
                           Log::Warn("request failed: {}", result.Error);
                   });
```

`HttpResult::Ok` reflects transport success only; check `StatusCode` for the HTTP verdict (or use
`IsSuccess` below).

## Config-driven JSON endpoints (RestJsonApi)

`JsonPostSpec` describes an endpoint entirely from settings: URL, optional auth header, and a JSON
body template whose `{token}` placeholders are substituted per call. This lets server operators
wire a plugin to their own backend without code changes.

```cpp
#include <CS2Kit/Http/RestJsonApi.hpp>
using namespace CS2Kit::Http;

JsonPostSpec spec{
    .Url = cfg.createRoomUrl,
    .ApiKey = cfg.apiKey,             // "" = no auth header
    .AuthHeader = "Authorization",
    .AuthScheme = "Bearer",           // "" sends the key verbatim
    .BodyTemplate = cfg.requestBody,  // nlohmann::json with {token} placeholders
    .TimeoutMs = 8000,
};

auto request = BuildJsonPost(spec, {{"steamId", std::to_string(steamId)}, {"playerName", name}});
if (request)
{
    Post(Engine().Http, std::move(*request), [](const HttpResult& result) {
        if (!IsSuccess(result))   // Ok && 2xx
            return;
        // Dot-path field extraction, with an optional {value} template:
        std::string url = ExtractField(result, "data.room.playerUrl");
    });
}
```

## Threading model

Requests run off-thread; **callbacks never run concurrently with game code**. They are queued and
invoked from the frame pump, so a completion may touch players, menus, chat, and any other engine
state without synchronization. Do not block on a request from the game thread - there is no
synchronous API by design.
