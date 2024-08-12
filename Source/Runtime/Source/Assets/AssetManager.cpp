#include <Assets/AssetManager.h>
#include <Log/Log.h>

#include <numeric>

namespace DX
{
    AssetManager::AssetManager()
    {
        DX_LOG(Info, "Asset Manager", "Initializing Asset Manager...");
    }

    AssetManager::~AssetManager()
    {
#ifndef NDEBUG
        int leakedAssets = std::reduce(m_assets.begin(), m_assets.end(), 0,
            [](int accumulator, const auto& asset)
            {
                return accumulator + (asset.second.use_count() > 1) ? 1 : 0;
            });
        if (leakedAssets > 0)
        {
            DX_LOG(Warning, "Device", "There are %d assets still referenced at the time of destroying asset manager.", leakedAssets);
        }
#endif

        m_assets.clear();

        DX_LOG(Info, "Asset Manager", "Terminating Asset Manager...");
    }

    void AssetManager::AddAsset(std::shared_ptr<AssetBase> asset)
    {
        m_assets.emplace(asset->GetAssetId(), asset);
    }

    void AssetManager::RemoveAsset(AssetId assetId)
    {
        // If there are no other references to the asset it'll be destroyed when removed from map.
        m_assets.erase(assetId);
    }

    std::shared_ptr<AssetBase> AssetManager::GetAsset(AssetId assetId)
    {
        if (auto it = m_assets.find(assetId);
            it != m_assets.end())
        {
            return it->second;
        }
        return {};
    }
} // namespace DX
