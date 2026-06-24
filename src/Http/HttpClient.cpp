#include <CS2Kit/Http/HttpClient.hpp>

// Static libcurl on Windows: must be declared before including curl.h or symbols resolve to dllimport.
#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif
#include <curl/curl.h>

namespace CS2Kit::Http
{

namespace
{
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    const size_t total = size * nmemb;
    static_cast<std::string*>(userdata)->append(ptr, total);
    return total;
}
}  // namespace

void HttpClient::Start()
{
    if (_running)
        return;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    _running = true;
    _worker = std::thread([this] { WorkerLoop(); });
}

void HttpClient::Stop()
{
    if (!_running)
        return;

    {
        std::lock_guard<std::mutex> lock(_jobMutex);
        _running = false;
    }
    _jobCv.notify_all();

    if (_worker.joinable())
        _worker.join();

    curl_global_cleanup();

    // Worker has joined: no other thread touches these queues now.
    std::queue<Job> emptyJobs;
    std::swap(_jobs, emptyJobs);
    _completions.clear();
}

void HttpClient::Post(std::string url, std::string body, std::vector<std::string> headers, long timeoutMs,
                      HttpCompletion onComplete)
{
    {
        std::lock_guard<std::mutex> lock(_jobMutex);
        if (!_running)
            return;
        _jobs.push(Job{std::move(url), std::move(body), std::move(headers), timeoutMs, std::move(onComplete)});
    }
    _jobCv.notify_one();
}

void HttpClient::DispatchCompletions()
{
    std::vector<Completion> ready;
    {
        std::lock_guard<std::mutex> lock(_completionMutex);
        if (_completions.empty())
            return;
        ready.swap(_completions);
    }

    for (auto& c : ready)
    {
        if (c.OnComplete)
            c.OnComplete(c.Result);
    }
}

void HttpClient::WorkerLoop()
{
    while (true)
    {
        Job job;
        {
            std::unique_lock<std::mutex> lock(_jobMutex);
            _jobCv.wait(lock, [this] { return !_running || !_jobs.empty(); });
            if (!_running)
                return;  // shutdown: drop any queued jobs, only the in-flight request (if any) finishes
            job = std::move(_jobs.front());
            _jobs.pop();
        }

        HttpResult result;
        CURL* curl = curl_easy_init();
        if (!curl)
        {
            result.Error = "curl_easy_init failed";
        }
        else
        {
            std::string response;
            curl_easy_setopt(curl, CURLOPT_URL, job.Url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, job.Body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(job.Body.size()));
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, job.TimeoutMs);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            curl_slist* headerList = nullptr;
            for (const auto& h : job.Headers)
                headerList = curl_slist_append(headerList, h.c_str());
            if (headerList)
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

            const CURLcode rc = curl_easy_perform(curl);
            if (rc == CURLE_OK)
            {
                long code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
                result.Ok = true;
                result.StatusCode = code;
                result.Body = std::move(response);
            }
            else
            {
                result.Error = curl_easy_strerror(rc);
            }

            if (headerList)
                curl_slist_free_all(headerList);
            curl_easy_cleanup(curl);
        }

        {
            std::lock_guard<std::mutex> lock(_completionMutex);
            _completions.push_back({std::move(job.OnComplete), std::move(result)});
        }
    }
}

}  // namespace CS2Kit::Http
