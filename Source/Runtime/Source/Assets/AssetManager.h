#pragma once

#include <Singleton/Singleton.h>
#include <Assets/Asset.h>
#include <File/FileUtils.h>
#include <Log/Log.h>

#include <memory>
#include <unordered_map>
#include <functional>

namespace DX
{
    template<typename T>
    using LoadDataFunc = std::function<std::unique_ptr<typename T::DataType>(const std::filesystem::path& fileNamePath)>;
    
    // Manager for all assets. It stores all assets in a map and provides methods to get them.
    // Each specific asset type will use AssetManager to load and store assets.
    // Do not use AssetManager directly, use the specific Asset class instead.
    class AssetManager : public Singleton<AssetManager>
    {
        friend class Singleton<AssetManager>;
        AssetManager();

    public:
        ~AssetManager();

        void AddAsset(std::shared_ptr<AssetBase> asset);
        void RemoveAsset(AssetId assetId);

        std::shared_ptr<AssetBase> GetAsset(AssetId assetId);

        template<typename T>
        std::shared_ptr<T> GetAssetAs(AssetId assetId);

        // Loads an asset from a file. The filename is relative to the Assets folder.
        template<typename T>
        std::shared_ptr<T> LoadAssetAs(const std::string& fileName, LoadDataFunc<T> loadDataFunc);

    private:
        using Assets = std::unordered_map<AssetId, std::shared_ptr<AssetBase>>;

        Assets m_assets;
    };

    template<typename T>
    std::shared_ptr<T> AssetManager::GetAssetAs(AssetId assetId)
    {
        if (auto it = m_assets.find(assetId);
            it != m_assets.end())
        {
            return std::static_pointer_cast<T>(it->second);
        }
        return {};
    }

    template<typename T>
    std::shared_ptr<T> AssetManager::LoadAssetAs(const std::string& fileName, LoadDataFunc<T> loadDataFunc)
    {
        if (fileName.empty())
        {
            DX_LOG(Error, "AssetManager", "Filename is empty.");
            return nullptr;
        }

        // Check if asset already exists (by Id)
        if (auto asset = GetAsset(fileName))
        {
            if (asset->GetAssetType() == T::AssetTypeId)
            {
                return std::static_pointer_cast<T>(asset);
            }
            else
            {
                DX_LOG(Error, "AssetManager", "An asset of different asset type already exists with Id %s.", fileName.c_str());
                return nullptr;
            }
        }

        // Check if filename exists
        auto fileNamePath = GetAssetPath() / fileName;
        if (!std::filesystem::exists(fileNamePath))
        {
            DX_LOG(Error, "AssetManager", "Filename path %s does not exist.", fileNamePath.generic_string().c_str());
            return nullptr;
        }

        auto data = loadDataFunc(fileNamePath);
        if (!data)
        {
            DX_LOG(Error, "AssetManager", "Failed to load asset %s.", fileNamePath.generic_string().c_str());
            return nullptr;
        }

        std::shared_ptr<T> newAsset(new T(fileName, std::move(data)));

        DX::AssetManager::Get().AddAsset(newAsset);

        return newAsset;
    }
} // namespace DX
