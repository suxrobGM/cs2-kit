#pragma once

#include "HttpResult.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace CS2Kit::Http
{

/**
 * Async HTTP client. A single worker thread performs blocking libcurl requests off the game
 * thread; completions are queued and replayed on the game thread via `DispatchCompletions()` so
 * callbacks may safely touch engine state. No engine API may be called from the completion before it.
 */
class HttpClient
{
public:
    HttpClient() = default;

    /** Init libcurl and spawn the worker thread. Idempotent. */
    void Start();

    /** Signal + join the worker, clean up libcurl, drop any unrun completions. Idempotent. */
    void Stop();

    /**
     * Enqueue an async POST. `headers` are full "Key: Value" lines. `onComplete` runs on the game
     * thread on a later `DispatchCompletions()`. No-op if the client isn't running.
     */
    void Post(std::string url, std::string body, std::vector<std::string> headers, long timeoutMs,
              HttpCompletion onComplete);

    /** Invoke all ready completions on the calling (game) thread. */
    void DispatchCompletions();

private:
    struct Job
    {
        std::string Url;
        std::string Body;
        std::vector<std::string> Headers;
        long TimeoutMs = 0;
        HttpCompletion OnComplete;
    };

    struct Completion
    {
        HttpCompletion OnComplete;
        HttpResult Result;
    };

    void WorkerLoop();

    std::thread _worker;
    bool _running = false;

    std::mutex _jobMutex;
    std::condition_variable _jobCv;
    std::queue<Job> _jobs;

    std::mutex _completionMutex;
    std::vector<Completion> _completions;
};

}  // namespace CS2Kit::Http
