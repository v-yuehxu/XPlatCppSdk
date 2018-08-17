#include <stdafx.h>

#include <playfab/PlayFabPluginManager.h>
#include <playfab/PlayFabHttp.h>

namespace PlayFab
{
    PlayFabPluginManager& PlayFabPluginManager::instance()
    {
        static PlayFabPluginManager sInstance;
        return sInstance;
    }

    void PlayFabPluginManager::SetPlugin(IPlayFabPlugin& plugin, const PlayFabPluginContract& contract, const std::string& instanceName)
    {
        instance().SetPluginInternal(plugin, contract, instanceName);
    }

    IPlayFabPlugin& PlayFabPluginManager::GetPluginInternal(const PlayFabPluginContract& contract, const std::string& instanceName)
    {
        const auto key = std::make_pair(contract, instanceName);
        auto pluginEntry = mPlugins.find(key);
        if (pluginEntry == mPlugins.end())
        {
            // Requested plugin is not in the cache, create the default one
            IPlayFabPlugin *pluginPtr = nullptr;
            switch (contract)
            {
            case PlayFabPluginContract::PlayFab_Serializer:
                pluginPtr = CreatePlayFabSerializerPlugin();
                break;
            case PlayFabPluginContract::PlayFab_Transport:
                pluginPtr = CreatePlayFabTransportPlugin();
                break;
            default:
                throw std::runtime_error("This contract is not supported");
                break;
            }

            mPlugins.insert({ key, *pluginPtr });
            return *pluginPtr;
        }
        else
        {
            return pluginEntry->second;
        }
    }

    void PlayFabPluginManager::SetPluginInternal(IPlayFabPlugin& plugin, const PlayFabPluginContract& contract, const std::string& instanceName)
    {
        const auto key = std::make_pair(contract, instanceName);
        auto pluginEntry = mPlugins.find(key);
        if (pluginEntry == mPlugins.end())
        {
            mPlugins.insert({ key, plugin });
        }
        else
        {
            mPlugins.erase(key);
            mPlugins.insert({ key, plugin });
        }
    }

    IPlayFabPlugin* PlayFabPluginManager::CreatePlayFabSerializerPlugin()
    {
        return new IPlayFabSerializerPlugin;
    }

    IPlayFabPlugin* PlayFabPluginManager::CreatePlayFabTransportPlugin()
    {
        return &IPlayFabHttp::Get();
    }
}
