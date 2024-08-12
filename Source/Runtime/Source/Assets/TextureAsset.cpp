#include <Assets/TextureAsset.h>
#include <Assets/AssetManager.h>
#include <Log/Log.h>

#include <stb_image.h>

namespace DX
{
    TextureAsset::TextureAsset(AssetId assetId, std::unique_ptr<TextureData> data)
        : Super(assetId, std::move(data))
    {
    }

    std::shared_ptr<TextureAsset> TextureAsset::LoadTextureAsset(const std::string& fileName)
    {
        return DX::AssetManager::Get().LoadAssetAs<TextureAsset>(
            fileName, 
            std::bind(&TextureAsset::LoadTexture, std::placeholders::_1));
    }

    std::unique_ptr<TextureData> TextureAsset::LoadTexture(const std::filesystem::path& fileNamePath)
    {
        auto textureData = std::make_unique<TextureData>();

        textureData->m_data = stbi_load(
            fileNamePath.generic_string().c_str(),
            &textureData->m_size.x,
            &textureData->m_size.y,
            nullptr,
            STBI_rgb_alpha);

        if (!textureData->m_data)
        {
            DX_LOG(Error, "TextureAsset", "Failed to load texture %s.", fileNamePath.generic_string().c_str());
            return nullptr;
        }

        return textureData;
    }
} // namespace DX
