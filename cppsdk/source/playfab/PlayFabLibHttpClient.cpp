#include <stdafx.h>

#include <playfab/PlayFabLibHttpClient.h>
#include <playfab/PlayFabSettings.h>

#include <stdexcept>

namespace PlayFab
{
    std::shared_ptr<IPlayFabLibHttpClient> IPlayFabLibHttpClient::httpInstance = nullptr;
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
        activeRequestCount = 0;
        threadRunning = true;
        pfHttpWorkerThread = std::thread(&PlayFabLibHttpClient::WorkerThread, this);
    };

    PlayFabLibHttpClient::~PlayFabLibHttpClient()
    {
        threadRunning = false;
        pfHttpWorkerThread.join();
        for (size_t i = 0; i < pendingRequests.size(); ++i)
        {
            delete pendingRequests[i];
        }

        pendingRequests.clear();

        for (size_t i = 0; i < pendingResults.size(); ++i)
        {
            delete pendingResults[i];
        }

        pendingResults.clear();

        activeRequestCount = 0;
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

    void PlayFabLibHttpClient::WorkerThread()
    {
        size_t queueSize;

        while (this->threadRunning)
        {
            CallRequestContainerBase* reqContainer = nullptr;

            { // LOCK httpRequestMutex
                std::unique_lock<std::mutex> lock(this->httpRequestMutex);

                queueSize = this->pendingRequests.size();
                if (queueSize != 0)
                {
                    reqContainer = this->pendingRequests[this->pendingRequests.size() - 1];
                    this->pendingRequests.pop_back();
                }
            } // UNLOCK httpRequestMutex

            if (queueSize == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            if (reqContainer != nullptr)
            {
                ExecuteRequest(*static_cast<CallRequestContainer*>(reqContainer));
            }
        }
    }

    void PlayFabLibHttpClient::HandleCallback(CallRequestContainer& reqContainer)
    {
        reqContainer.finished = true;
        if (PlayFabSettings::threadedCallbacks)
        {
            HandleResults(reqContainer);
        }

        if (!PlayFabSettings::threadedCallbacks)
        {
            PlayFabLibHttpClient& instance = static_cast<PlayFabLibHttpClient&>(Get());
            { // LOCK httpRequestMutex
                std::unique_lock<std::mutex> lock(instance.httpRequestMutex);
                instance.pendingResults.push_back(&reqContainer);
            } // UNLOCK httpRequestMutex
        }
    }

    void PlayFabLibHttpClient::MakePostRequest(const CallRequestContainerBase& reqContainer)
    {
        { // LOCK httpRequestMutex
            std::unique_lock<std::mutex> lock(httpRequestMutex);
            pendingRequests.push_back(const_cast<CallRequestContainerBase*>(&reqContainer));
            activeRequestCount++;
        } // UNLOCK httpRequestMutex
    }

    void PlayFabLibHttpClient::SetupRequestHeader(CallRequestContainer& reqContainer, std::vector<std::vector<std::string>>& outHeaders)
    {
        std::vector<std::string> header;
        header.clear();
        //header.push_back(nullptr);
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

    void PlayFabLibHttpClient::HttpCallPerformCallback(AsyncBlock * asyncBlock)
    {
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

    void PlayFabLibHttpClient::ExecuteRequest(CallRequestContainer& reqContainer)
    {
        std::string method = "POST";
        std::string url = PlayFabSettings::GetUrl(reqContainer.GetUrl(), PlayFabSettings::requestGetParams);
        std::string requestBody = reqContainer.GetRequestBody();

        bool retryAllowed = true;
        
        std::vector<std::vector<std::string>> headers;
        SetupRequestHeader(reqContainer, headers);

        HCInitialize(nullptr);

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
        asyncBlock->context = reinterpret_cast<void*>(&context);
        asyncBlock->callback = HttpCallPerformCallback;
    
        HCHttpCallPerformAsync(call, asyncBlock);
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

    size_t PlayFabLibHttpClient::Update()
    {
        if (PlayFabSettings::threadedCallbacks)
        {
            throw std::runtime_error("You should not call Update() when PlayFabSettings::threadedCallbacks == true");
        }

        CallRequestContainerBase* reqContainer;
        { // LOCK httpRequestMutex
            std::unique_lock<std::mutex> lock(httpRequestMutex);
            if (pendingResults.empty())
            {
                return activeRequestCount;
            }

            reqContainer = pendingResults[pendingResults.size() - 1];
            pendingResults.pop_back();
            activeRequestCount--;
        } // UNLOCK httpRequestMutex

          // The callback called from HandleResults may delete the object pointed by reqContainer from the heap; do not use it after this call
        HandleResults(*static_cast<CallRequestContainer*>(reqContainer));

        // activeRequestCount can be altered by HandleResults, so we have to re-lock and return an updated value
        { // LOCK httpRequestMutex
            std::unique_lock<std::mutex> lock(httpRequestMutex);
            return activeRequestCount;
        }
    }
}