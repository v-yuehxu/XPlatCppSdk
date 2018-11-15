#include <stdafx.h>

#include <playfab/PlayFabLibHttpClient.h>
#include <playfab/PlayFabSettings.h>

#include <stdexcept>

namespace PlayFab
{
    std::shared_ptr<IPlayFabLibHttpClient> IPlayFabLibHttpClient::httpInstance = nullptr;
    async_queue_handle_t PlayFabLibHttpClient::queue = nullptr;
    registration_token_t PlayFabLibHttpClient::callbackToken = 0;

    win32_handle PlayFabLibHttpClient::stopRequestedHandle;
    win32_handle PlayFabLibHttpClient::workReadyHandle;
    win32_handle PlayFabLibHttpClient::completionReadyHandle;
    win32_handle PlayFabLibHttpClient::exampleTaskDone;

    IPlayFabLibHttpClient::~IPlayFabLibHttpClient() = default;
    IPlayFabLibHttpClient& IPlayFabLibHttpClient::Get()
    {
        return *GetPtr().get();
    }

    std::shared_ptr<IPlayFabLibHttpClient> IPlayFabLibHttpClient::GetPtr()
    {
        // In the future we could make it easier to override this instance with a sub-type, for now it defaults to the only one we have
        if (httpInstance == nullptr)
        {
            PlayFabLibHttpClient::MakeInstance();
        }

        return httpInstance;
    }

    PlayFabLibHttpClient::PlayFabLibHttpClient()
    {
        /*activeRequestCount = 0;
        threadRunning = true;*/
        // pfHttpWorkerThread = std::thread(&PlayFabLibHttpClient::WorkerThread, this);

        targetNumThreads = 2;
        hActiveThreads[10] = { 0 };
        defaultIdealProcessor = 0;
        numActiveThreads = 0;

        //// todo sangarg : These 2 need to be done here. Currently the issue is that the exampleTaskDone handle is not setup correctly so making
        //// multiple calls becomes an issue.
        //InitializeLibHttpClient();
        //StartBackgroundThread();
    };

    PlayFabLibHttpClient::~PlayFabLibHttpClient()
    {
        //// todo sangarg : These 2 should be done here. Currenly the handles are not setup correctly hence we need to call them in the MakePostRequest call
        //ShutdownActiveThreads();
        //HCCleanup();

        //threadRunning = false;
        //// pfHttpWorkerThread.join();
        //for (size_t i = 0; i < pendingRequests.size(); ++i)
        //{
        //    delete pendingRequests[i];
        //}

        //pendingRequests.clear();

        //for (size_t i = 0; i < pendingResults.size(); ++i)
        //{
        //    delete pendingResults[i];
        //}

        //pendingResults.clear();

        //activeRequestCount = 0;
    }

    void PlayFabLibHttpClient::MakeInstance()
    {
        if (httpInstance == nullptr)
        {
#if __cplusplus > 201103L
            class _PlayFabLibHttpClient : public PlayFabLibHttpClient {}; // Hack to bypass private constructor on PlayFabHttp
            httpInstance = std::make_shared<_PlayFabLibHttpClient>();
#else
#pragma message("PlayFab SDK: C++11 support is not well tested. Please consider upgrading to the latest version of Visual Studio")
            httpInstance = std::shared_ptr<PlayFabLibHttpClient>(new PlayFabLibHttpClient());
#endif
        }
    }

    void PlayFabLibHttpClient::InitializeLibHttpClient()
    {
        auto err = HCInitialize(nullptr);

        if (err != S_OK)
        {
            // todo : handle errors and decide what to do
            // Note that this is called in the constructor so not sure what we can do here.
        }

        uint32_t sharedAsyncQueueId = 0;
        err = CreateSharedAsyncQueue(
            sharedAsyncQueueId,
            AsyncQueueDispatchMode::AsyncQueueDispatchMode_Manual,
            AsyncQueueDispatchMode::AsyncQueueDispatchMode_Manual,
            &queue);
        
        err = RegisterAsyncQueueCallbackSubmitted(queue, nullptr, HandleAsyncQueueCallback, &callbackToken);
    }

    void PlayFabLibHttpClient::StartBackgroundThread()
    {
        stopRequestedHandle.set(CreateEvent(nullptr, true, false, nullptr));
        workReadyHandle.set(CreateEvent(nullptr, false, false, nullptr));
        completionReadyHandle.set(CreateEvent(nullptr, false, false, nullptr));
        exampleTaskDone.set(CreateEvent(nullptr, false, false, nullptr));

        for (uint32_t i = 0; i < targetNumThreads; i++)
        {
            hActiveThreads[i] = CreateThread(nullptr, 0, BackgroundThreadProc, nullptr, 0, nullptr);
            if (defaultIdealProcessor != MAXIMUM_PROCESSORS)
            {
                if (hActiveThreads[i] != nullptr)
                {
                    SetThreadIdealProcessor(hActiveThreads[i], defaultIdealProcessor);
                }
            }
        }

        numActiveThreads = targetNumThreads;
    }

    DWORD WINAPI PlayFabLibHttpClient::BackgroundThreadProc(LPVOID lpParam)
    {
        HANDLE hEvents[3] =
        {
            workReadyHandle.get(),
            completionReadyHandle.get(),
            stopRequestedHandle.get()
        };

        async_queue_handle_t queue;
        uint32_t sharedAsyncQueueId = 0;
        CreateSharedAsyncQueue(
            sharedAsyncQueueId,
            AsyncQueueDispatchMode::AsyncQueueDispatchMode_Manual,
            AsyncQueueDispatchMode::AsyncQueueDispatchMode_Manual,
            &queue);

        bool stop = false;
        while (!stop)
        {
            DWORD dwResult = WaitForMultipleObjectsEx(3, hEvents, false, INFINITE, false);
            switch (dwResult)
            {
            case WAIT_OBJECT_0: // work ready
                DispatchAsyncQueue(queue, AsyncQueueCallbackType_Work, 0);

                if (!IsAsyncQueueEmpty(queue, AsyncQueueCallbackType_Work))
                {
                    // If there's more pending work, then set the event to process them
                    SetEvent(workReadyHandle.get());
                }
                break;

            case WAIT_OBJECT_0 + 1: // completed 
                                    // Typically completions should be dispatched on the game thread, but
                                    // for this simple XAML app we're doing it here
                DispatchAsyncQueue(queue, AsyncQueueCallbackType_Completion, 0);

                if (!IsAsyncQueueEmpty(queue, AsyncQueueCallbackType_Completion))
                {
                    // If there's more pending completions, then set the event to process them
                    SetEvent(completionReadyHandle.get());
                }
                break;

            default:
                stop = true;
                break;
            }
        }

        return 0;
    }

    void  PlayFabLibHttpClient::HandleAsyncQueueCallback(_In_ void* context, _In_ async_queue_handle_t queue, _In_ AsyncQueueCallbackType type)
    {
        UNREFERENCED_PARAMETER(context);
        UNREFERENCED_PARAMETER(queue);

        switch (type)
        {
        case AsyncQueueCallbackType::AsyncQueueCallbackType_Work:
            SetEvent(workReadyHandle.get());
            break;

        case AsyncQueueCallbackType::AsyncQueueCallbackType_Completion:
            SetEvent(completionReadyHandle.get());
            break;
        }
    }

    void PlayFabLibHttpClient::MakePostRequest(const CallRequestContainerBase& reqContainer)
    {
        // Create the request and call HCHttpCallPerformAsync

        std::string method = "GET";

        std::string url = "https://raw.githubusercontent.com/Microsoft/libHttpClient/master/Samples/Win32-Http/TestContent.json";
        std::string requestBody = "";// "{\"test\":\"value\"},{\"test2\":\"value\"},{\"test3\":\"value\"},{\"test4\":\"value\"},{\"test5\":\"value\"},{\"test6\":\"value\"},{\"test7\":\"value\"}";
        bool retryAllowed = true;
        std::vector<std::vector<std::string>> headers;
        std::vector< std::string > header;

        header.clear();
        header.push_back("TestHeader");
        header.push_back("1.0");
        headers.push_back(header);

        InitializeLibHttpClient();
        StartBackgroundThread();

        hc_call_handle_t call = nullptr;
        HCHttpCallCreate(&call);
        HCHttpCallRequestSetUrl(call, method.c_str(), url.c_str());
        HCHttpCallRequestSetRequestBodyString(call, requestBody.c_str());
        HCHttpCallRequestSetRetryAllowed(call, retryAllowed);
        for (auto& header : headers)
        {
            std::string headerName = header[0];
            std::string headerValue = header[1];
            HCHttpCallRequestSetHeader(call, headerName.c_str(), headerValue.c_str(), true);
        }

        printf_s("Calling %s %s\r\n", method.c_str(), url.c_str());

        AsyncBlock* asyncBlock = new AsyncBlock;
        ZeroMemory(asyncBlock, sizeof(AsyncBlock));
        asyncBlock->context = call;
        asyncBlock->queue = queue;
        asyncBlock->callback = HandleHttpCallPerformCallback;

        HCHttpCallPerformAsync(call, asyncBlock);

        WaitForSingleObject(exampleTaskDone.get(), INFINITE);

        ShutdownActiveThreads();
        HCCleanup();
    }

    // todo sangarg : Remove this function. This is only used for the output in the sample's HandleHttpCallPerformCallback
    std::vector<std::vector<std::string>> ExtractAllHeaders(_In_ hc_call_handle_t call)
    {
        uint32_t numHeaders = 0;
        HCHttpCallResponseGetNumHeaders(call, &numHeaders);

        std::vector< std::vector<std::string> > headers;
        for (uint32_t i = 0; i < numHeaders; i++)
        {
            const char* str;
            const char* str2;
            std::string headerName;
            std::string headerValue;
            HCHttpCallResponseGetHeaderAtIndex(call, i, &str, &str2);
            if (str != nullptr) headerName = str;
            if (str2 != nullptr) headerValue = str2;
            std::vector<std::string> header;
            header.push_back(headerName);
            header.push_back(headerValue);

            headers.push_back(header);
        }

        return headers;
    }

    void PlayFabLibHttpClient::HandleHttpCallPerformCallback(_Inout_ struct AsyncBlock * asyncBlock)
    {
        const char* str;
        HRESULT errCode = S_OK;
        uint32_t platErrCode = 0;
        uint32_t statusCode = 0;
        std::string responseString;
        std::string errMessage;

        hc_call_handle_t call = static_cast<hc_call_handle_t>(asyncBlock->context);
        HCHttpCallResponseGetNetworkErrorCode(call, &errCode, &platErrCode);
        HCHttpCallResponseGetStatusCode(call, &statusCode);
        HCHttpCallResponseGetResponseString(call, &str);
        if (str != nullptr) responseString = str;
        std::vector<std::vector<std::string>> headers = ExtractAllHeaders(call);

        HCHttpCallCloseHandle(call);

        printf_s("HTTP call done\r\n");
        printf_s("Network error code: %d\r\n", errCode);
        printf_s("HTTP status code: %d\r\n", statusCode);

        int i = 0;
        for (auto& header : headers)
        {
            printf_s("Header[%d] '%s'='%s'\r\n", i, header[0].c_str(), header[1].c_str());
            i++;
        }

        if (responseString.length() > 0)
        {
            // Returned string starts with a BOM strip it out.
            uint8_t BOM[] = { 0xef, 0xbb, 0xbf, 0x0 };
            if (responseString.find(reinterpret_cast<char*>(BOM)) == 0)
            {
                responseString = responseString.substr(3);
            }
            //                web::json::value json = web::json::value::parse(utility::conversions::to_string_t(responseString));;
        }

        if (responseString.length() > 200)
        {
            std::string subResponseString = responseString.substr(0, 200);
            printf_s("Response string:\r\n%s...\r\n", subResponseString.c_str());
        }
        else
        {
            printf_s("Response string:\r\n%s\r\n", responseString.c_str());
        }

        SetEvent(exampleTaskDone.get());
        delete asyncBlock;
    }

    void PlayFabLibHttpClient::ShutdownActiveThreads()
    {
        SetEvent(stopRequestedHandle.get());
        DWORD dwResult = WaitForMultipleObjectsEx(numActiveThreads, hActiveThreads, true, INFINITE, false);
        if (dwResult >= WAIT_OBJECT_0 && dwResult <= WAIT_OBJECT_0 + numActiveThreads - 1)
        {
            for (DWORD i = 0; i < numActiveThreads; i++)
            {
                CloseHandle(hActiveThreads[i]);
                hActiveThreads[i] = nullptr;
            }
            numActiveThreads = 0;
            ResetEvent(stopRequestedHandle.get());
        }
    }

    size_t PlayFabLibHttpClient::Update()
    {
        if (PlayFabSettings::threadedCallbacks)
        {
            throw std::runtime_error("You should not call Update() when PlayFabSettings::threadedCallbacks == true");
        }

        // todo sangarg: Figure out what this code is doing and remove the use of httpRequestMutex if possible

        //CallRequestContainerBase* reqContainer;
        //{ // LOCK httpRequestMutex
        //    std::unique_lock<std::mutex> lock(httpRequestMutex);
        //    if (pendingResults.empty())
        //    {
        //        return activeRequestCount;
        //    }

        //    reqContainer = pendingResults[pendingResults.size() - 1];
        //    pendingResults.pop_back();
        //    activeRequestCount--;
        //} // UNLOCK httpRequestMutex

        //  // The callback called from HandleResults may delete the object pointed by reqContainer from the heap; do not use it after this call
        //HandleResults(*static_cast<CallRequestContainer*>(reqContainer));

        //// activeRequestCount can be altered by HandleResults, so we have to re-lock and return an updated value
        //{ // LOCK httpRequestMutex
        //    std::unique_lock<std::mutex> lock(httpRequestMutex);
        //    return activeRequestCount;
        //}
    }



// =====================================================================================================================================================
//                      Code specific to making playfab related calls
// =====================================================================================================================================================


    void PlayFabLibHttpClient::HandleCallback(CallRequestContainer& reqContainer)
    {
        reqContainer.finished = true;
        if (PlayFabSettings::threadedCallbacks)
        {
            HandleResults(reqContainer);
        }

        // todo sangarg : Figure out if we need the threadCallbacks and what they are doing. We might need to bring back the httpRequestMutex in that case

        //if (!PlayFabSettings::threadedCallbacks)
        //{
        //    PlayFabLibHttpClient& instance = static_cast<PlayFabLibHttpClient&>(Get());
        //    { // LOCK httpRequestMutex
        //        std::unique_lock<std::mutex> lock(instance.httpRequestMutex);
        //        instance.pendingResults.push_back(&reqContainer);
        //    } // UNLOCK httpRequestMutex
        //}
    }

    void PlayFabLibHttpClient::SetupRequestHeader(CallRequestContainer& reqContainer, std::vector<std::vector<std::string>>& outHeaders)
    {
        std::vector<std::string> header;
        header.clear();
        header.push_back("Accept: application/json");
        header.push_back("Content-Type: application/json; charset=utf-8");
        header.push_back(("X-PlayFabSDK: " + PlayFabSettings::versionString).c_str());
        header.push_back("X-ReportErrorAsSuccess: true");

        auto headers = reqContainer.GetHeaders();

        if (headers.size() > 0)
        {
            for (auto const &obj : headers)
            {
                if (obj.first.length() != 0 && obj.second.length() != 0) // no empty keys or values in headers
                {
                    std::string header1 = obj.first + ": " + obj.second;
                    header.push_back(header1);
                }
            }
        }

        outHeaders.push_back(std::move(header));
    }

    void PlayFabLibHttpClient::PLAYFAB_MakePostRequest(CallRequestContainer& reqContainer)
    {
        // todo sangarg [NOTE] : This function makes the POST request to playfab. THIS HAS NOT BEEN TESTED so the code might need to change here.
        // This is not called anywhere currently, this would be called from MakePostRequest, or this code can actually be copied into MakePostRequest
        std::string method = "POST";
        std::string url = PlayFabSettings::GetUrl(reqContainer.GetUrl(), PlayFabSettings::requestGetParams);
        std::string requestBody = reqContainer.GetRequestBody();

        bool retryAllowed = true;
        
        std::vector<std::vector<std::string>> headers;
        SetupRequestHeader(reqContainer, headers);

        HCInitialize(nullptr);

        uint32_t sharedAsyncQueueId = 0;
        CreateSharedAsyncQueue(
            sharedAsyncQueueId,
            AsyncQueueDispatchMode::AsyncQueueDispatchMode_Manual,
            AsyncQueueDispatchMode::AsyncQueueDispatchMode_Manual,
            &queue);
        RegisterAsyncQueueCallbackSubmitted(queue, nullptr, HandleAsyncQueueCallback, &callbackToken);

        hc_call_handle_t call = nullptr;
        HCHttpCallCreate(&call);
        HCHttpCallRequestSetUrl(call, method.c_str(), url.c_str());
        HCHttpCallRequestSetRequestBodyString(call, requestBody.c_str());
        HCHttpCallRequestSetRetryAllowed(call, retryAllowed);
        for (auto& header : headers)
        {
            std::string headerName = header[0];
            std::string headerValue = header[1];
            HCHttpCallRequestSetHeader(call, headerName.c_str(), headerValue.c_str(), true);
        }

        CallbackContext context;
        context.call = call;
        context.reqContainer = &reqContainer;

        AsyncBlock* asyncBlock = new AsyncBlock;
        ZeroMemory(asyncBlock, sizeof(AsyncBlock));
        asyncBlock->context = call;
        asyncBlock->queue = queue;
        asyncBlock->callback = HandleHttpCallPerformCallback;
    
        auto err = HCHttpCallPerformAsync(call, asyncBlock);

        if (err != S_OK)
        {
            // todo sangarg : Handle error cases here and other places as well.
            std::cout << "SomeError";
        }
    }

    void PlayFabLibHttpClient::PLAYFAB_HandleHttpCallPerformCallback(_Inout_ struct AsyncBlock * asyncBlock)
    {
        // todo sangarg [NOTE] : The code below is the callback for the actual PlayFab POST call. IT HAS NOT BEEN TESTS YET

        std::cout << "HandleHttpCallPerformCallback\n";
        const CHAR* str;
        HRESULT errCode = S_OK;
        uint32_t platErrCode = 0;
        uint32_t statusCode = 0;
        std::string errMessage;

        CallbackContext *context = reinterpret_cast<CallbackContext*>(asyncBlock->context);

        hc_call_handle_t call = context->call;
        CallRequestContainer* reqContainer = context->reqContainer;

        HCHttpCallResponseGetNetworkErrorCode(call, &errCode, &platErrCode);
        HCHttpCallResponseGetStatusCode(call, &statusCode);
        HCHttpCallResponseGetResponseString(call, &str);
        if (str != nullptr) reqContainer->responseString = str;

        HCHttpCallCloseHandle(call);

        if (errCode != S_OK)
        {
        reqContainer->errorWrapper.HttpCode = 408;
        reqContainer->errorWrapper.HttpStatus = "Failed to contact server";
        reqContainer->errorWrapper.ErrorCode = PlayFabErrorConnectionTimeout;
        reqContainer->errorWrapper.ErrorName = "Failed to contact server";
        reqContainer->errorWrapper.ErrorMessage = "Failed to contact server, error: " + std::to_string(errCode);
        HandleCallback(*reqContainer);
        }
        else
        {
            Json::CharReaderBuilder jsonReaderFactory;
            std::unique_ptr<Json::CharReader> jsonReader(jsonReaderFactory.newCharReader());
            JSONCPP_STRING jsonParseErrors;
            const bool parsedSuccessfully = jsonReader->parse(reqContainer->responseString.c_str(), reqContainer->responseString.c_str() + reqContainer->responseString.length(), &reqContainer->responseJson, &jsonParseErrors);

            if (parsedSuccessfully)
            {
                reqContainer->errorWrapper.HttpCode = reqContainer->responseJson.get("code", Json::Value::null).asInt();
                reqContainer->errorWrapper.HttpStatus = reqContainer->responseJson.get("status", Json::Value::null).asString();
                reqContainer->errorWrapper.Data = reqContainer->responseJson.get("data", Json::Value::null);
                reqContainer->errorWrapper.ErrorName = reqContainer->responseJson.get("error", Json::Value::null).asString();
                reqContainer->errorWrapper.ErrorMessage = reqContainer->responseJson.get("errorMessage", Json::Value::null).asString();
                reqContainer->errorWrapper.ErrorDetails = reqContainer->responseJson.get("errorDetails", Json::Value::null);
            }
            else
            {
                reqContainer->errorWrapper.HttpCode = 408;
                reqContainer->errorWrapper.HttpStatus = reqContainer->responseString;
                reqContainer->errorWrapper.ErrorCode = PlayFabErrorConnectionTimeout;
                reqContainer->errorWrapper.ErrorName = "Failed to parse PlayFab response";
                reqContainer->errorWrapper.ErrorMessage = jsonParseErrors;
            }

            HandleCallback(*reqContainer);
        }

        delete asyncBlock;
    }

    void PlayFabLibHttpClient::HandleResults(CallRequestContainer& reqContainer)
    {
        auto callback = reqContainer.GetCallback();
        if (callback != nullptr)
        {
            callback(
                reqContainer.responseJson.get("code", Json::Value::null).asInt(),
                reqContainer.responseString,
                reqContainer);
        }
    }
}