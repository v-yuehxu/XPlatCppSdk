#pragma once

#include <playfab/PlayFabError.h>
#include <unordered_map>

#include <curl/curl.h>

namespace PlayFab
{
    struct CallRequestContainer;
    typedef void(*RequestCompleteCallback)(CallRequestContainer& reqContainer);
    typedef std::shared_ptr<void> SharedVoidPointer;

    /// <summary>
    /// Internal PlayFabHttp container for each api call
    /// </summary>
    struct CallRequestContainer
    {
        // I own these objects, I must always destroy them
        CURL* curlHandle;
        curl_slist* curlHttpHeaders;
        // I never own this, I can never destroy it
        void* customData;

        bool finished;
        std::string authKey;
        std::string authValue;
        std::string responseString;
        Json::Value responseJson = Json::Value::null;
        PlayFabError errorWrapper;
        RequestCompleteCallback internalCallback;
        SharedVoidPointer successCallback;
        ErrorCallback errorCallback;

        CallRequestContainer();
        ~CallRequestContainer();
    };

    /// <summary>
    /// The enumeration of supported plugin contracts.
    /// </summary>
    enum class PlayFabPluginContract
    {
        PlayFab_Serializer,
        PlayFab_Transport
    };

    /// <summary>
    /// Interface of any PlayFab SDK plugin.
    /// </summary>
    class IPlayFabPlugin
    {
    };

    /// <summary>
    /// Interface of any transport SDK plugin.
    /// </summary>
    class IPlayFabTransportPlugin : public IPlayFabPlugin
    {
        virtual void AddRequest(const std::string&, const std::string&, const std::string&, const Json::Value&, RequestCompleteCallback, SharedVoidPointer, ErrorCallback, void*) = 0;
    };

    /// <summary>
    /// Interface of any data serializer SDK plugin.
    /// </summary>
    class IPlayFabSerializerPlugin : public IPlayFabPlugin
    {
    };

    /// <summary>
    /// The PlayFab plugin manager.
    /// </summary>
    class PlayFabPluginManager
    {
    public:
        static PlayFabPluginManager& instance(); // The singleton instance of plugin manager

        // Prevent copy/move construction
        PlayFabPluginManager(const PlayFabPluginManager&) = delete;
        PlayFabPluginManager(PlayFabPluginManager&&) = delete;

        // Prevent copy/move assignment operations
        PlayFabPluginManager& operator=(const PlayFabPluginManager&) = delete;
        PlayFabPluginManager& operator=(PlayFabPluginManager&&) = delete;

        // Gets a plugin.
        // If a plugin with specified contract and optional instance name does not exist, it will create a new one.
        template <typename T>
        static T& GetPlugin(const PlayFabPluginContract& contract, const std::string& instanceName = "") 
        { 
            return (T&)(instance().GetPluginInternal(contract, instanceName)); 
        }

        // Sets a custom plugin.
        // If a plugin with specified contract and optional instance name already exists, it will be replaced with specified instance.
        static void SetPlugin(IPlayFabPlugin& plugin, const PlayFabPluginContract& contract, const std::string& instanceName = "");
    private:
        IPlayFabPlugin& GetPluginInternal(const PlayFabPluginContract& contract, const std::string& instanceName);
        void SetPluginInternal(IPlayFabPlugin& plugin, const PlayFabPluginContract& contract, const std::string& instanceName);

        IPlayFabPlugin* CreatePlayFabSerializerPlugin();
        IPlayFabPlugin* CreatePlayFabTransportPlugin();

        // Private constructor and destructor
        PlayFabPluginManager() = default;
        ~PlayFabPluginManager() = default;

        std::map<const std::pair<const PlayFabPluginContract, const std::string>, IPlayFabPlugin&> mPlugins;
    };
}
