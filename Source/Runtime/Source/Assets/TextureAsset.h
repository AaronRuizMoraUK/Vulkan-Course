#pragma once

#include <Assets/Asset.h>
#include <Math/Vector2.h>

namespace std::filesystem
{
    class path;
}

namespace DX
{
    struct TextureData
    {
        Math::Vector2Int m_size;
        uint8_t* m_data;
    };

    // Texture formats supported: jpeg, png, bmp, psd, tga, gif, hdr, pic, and pnm
    class TextureAsset : public Asset<TextureData>
    {
    public:
        // Loads a texture from a file. The filename is relative to the assets folder.
        static std::shared_ptr<TextureAsset> LoadTextureAsset(const std::string& fileName);

        static inline const AssetType AssetTypeId = 0xB8FCE1BE;

        AssetType GetAssetType() const override
        {
            return AssetTypeId;
        }

    protected:
        friend class AssetManager;
        using Super = Asset<TextureData>;

        TextureAsset(AssetId assetId, std::unique_ptr<TextureData> data);

    private:
        static std::unique_ptr<TextureData> LoadTexture(const std::filesystem::path& fileNamePath);
    };
} // namespace DX
