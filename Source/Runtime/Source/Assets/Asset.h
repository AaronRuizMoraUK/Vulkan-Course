#pragma once

#include <string>
#include <memory>
#include <cstdint>

namespace DX
{
    using AssetId = std::string;

    using AssetType = uint32_t;

    // Base asset class with asset id and type.
    class AssetBase
    {
    public:
        virtual ~AssetBase() = default;

        AssetBase(const AssetBase&) = delete;
        AssetBase& operator=(const AssetBase&) = delete;

        const AssetId& GetAssetId() const
        {
            return m_assetId;
        }

        bool IsAssetIdValid() const
        {
            return m_assetId != "";
        }

        virtual AssetType GetAssetType() const = 0;

    protected:
        AssetBase(AssetId assetId)
            : m_assetId(assetId)
        {
        }

    private:
        AssetId m_assetId;
    };

    // Templated asset class with data.
    // All specific assets must derive from this.
    template<typename T>
    class Asset : public AssetBase
    {
    public:
        using DataType = T;

        const DataType* GetData() const
        {
            return m_data.get();
        }

    protected:
        Asset(AssetId assetId, std::unique_ptr<DataType> data)
            : AssetBase(assetId)
            , m_data(std::move(data))
        {
        }

        std::unique_ptr<DataType> m_data;
    };
} // namespace DX
