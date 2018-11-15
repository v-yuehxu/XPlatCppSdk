#pragma once

#include <playfab/PlayFabCallRequestContainer.h>
#include <playfab/PlayFabPluginManager.h>
#include <playfab/PlayFabError.h>

#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

#ifndef _WIN32
#include <jsoncpp/json/value.h>
#endif
#ifdef _WIN32
#include <json/value.h>
#endif

#include <httpClient/httpClient.h>

namespace PlayFab
{
    class win32_handle
    {
    public:
        win32_handle() : m_handle(nullptr)
        {
        }

        ~win32_handle()
        {
            if (m_handle != nullptr) CloseHandle(m_handle);
            m_handle = nullptr;
        }

        void set(HANDLE handle)
        {
            m_handle = handle;
        }

        HANDLE get() { return m_handle; }

    private:
        HANDLE m_handle;
    };

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

        void InitializeLibHttpClient();
        void StartBackgroundThread();
        static DWORD WINAPI BackgroundThreadProc(LPVOID lpParam);
        static void CALLBACK HandleAsyncQueueCallback(_In_ void* context, _In_ async_queue_handle_t queue, _In_ AsyncQueueCallbackType type);

        static void CALLBACK HandleHttpCallPerformCallback(_Inout_ struct AsyncBlock* asyncBlock);

        void ShutdownActiveThreads();


        // todo sangarg : The following functions are needed once we implement a good structure to make multiple calls
        // Once we can make multiple calls with the current MakePostRequest code, we rename 
        // PLAYFAB_MakePostRequest -> MakePostRequest and 
        // PLAYFAB_HandleHttpCallPerformCallback -> HandleHttpCallPerformCallback

        static void SetupRequestHeader(CallRequestContainer& reqContainer, std::vector<std::vector<std::string>>& outHeaders);
        static void PLAYFAB_MakePostRequest(CallRequestContainer& reqContainer);
        static void CALLBACK PLAYFAB_HandleHttpCallPerformCallback(_Inout_ struct AsyncBlock* asyncBlock);

        static void HandleCallback(CallRequestContainer& reqContainer);
        static void HandleResults(CallRequestContainer& reqContainer);

        /*std::mutex httpRequestMutex;
        bool threadRunning;
        int activeRequestCount;
        std::vector<CallRequestContainerBase*> pendingRequests;
        std::vector<CallRequestContainerBase*> pendingResults;*/


    private:
        static async_queue_handle_t queue;
        static registration_token_t callbackToken;

        static win32_handle stopRequestedHandle;
        static win32_handle workReadyHandle;
        static win32_handle completionReadyHandle;
        static win32_handle exampleTaskDone;

        DWORD targetNumThreads;
        HANDLE hActiveThreads[10];
        DWORD defaultIdealProcessor;
        DWORD numActiveThreads;

        /*static std::atomic<int> inFlightRequestCount;
        std::atomic<bool> acceptMoreRequests;*/
    };

    struct CallbackContext
    {
        hc_call_handle_t call;
        CallRequestContainer* reqContainer;
    };
}
