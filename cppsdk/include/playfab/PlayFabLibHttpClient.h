#pragma once

#include <playfab/PlayFabCallRequestContainer.h>
#include <playfab/PlayFabPluginManager.h>
#include <playfab/PlayFabError.h>

#include <functional>
#include <memory>
#include <thread>
#include <mutex>

#ifndef _WIN32
#include <jsoncpp/json/value.h>
#endif
#ifdef _WIN32
#include <json/value.h>
#endif

#include <httpClient/httpClient.h>

namespace PlayFab
{
    class IPlayFabLibHttpClient : public IPlayFabHttpPlugin
    {
    public:
        static IPlayFabLibHttpClient& Get();
        static std::shared_ptr<IPlayFabLibHttpClient> GetPtr();

        virtual ~IPlayFabLibHttpClient() = 0;

        virtual size_t Update() = 0;

        virtual void MakePostRequest(const CallRequestContainerBase&) = 0;

    protected:
        static std::shared_ptr<IPlayFabLibHttpClient> httpInstance;
    };

    class PlayFabLibHttpClient : public IPlayFabLibHttpClient
    {
    public:
        static void MakeInstance();
        ~PlayFabLibHttpClient() override;

        virtual void MakePostRequest(const CallRequestContainerBase&) override;

        size_t Update() override;

    private:
        PlayFabLibHttpClient(); // Private constructor, to enforce singleton instance
        PlayFabLibHttpClient(const PlayFabLibHttpClient& other); // Private copy-constructor, to enforce singleton instance

        PlayFabLibHttpClient(PlayFabLibHttpClient&& other) = delete;
        PlayFabLibHttpClient& operator=(PlayFabLibHttpClient&& other) = delete;

        static void ExecuteRequest(CallRequestContainer& reqContainer);
        void WorkerThread();
        static void HandleCallback(CallRequestContainer& reqContainer);
        static void HandleResults(CallRequestContainer& reqContainer);

        std::thread pfHttpWorkerThread;
        std::mutex httpRequestMutex;
        bool threadRunning;
        int activeRequestCount;
        std::vector<CallRequestContainerBase*> pendingRequests;
        std::vector<CallRequestContainerBase*> pendingResults;

        static void SetupRequestHeader(CallRequestContainer& reqContainer, std::vector<std::vector<std::string>>& outHeaders);
        static void HttpCallPerformCallback(_Inout_ struct AsyncBlock* asyncBlock);
    };

    struct CallbackContext
    {
        hc_call_handle_t call;
        CallRequestContainer* reqContainer;
    };
}
