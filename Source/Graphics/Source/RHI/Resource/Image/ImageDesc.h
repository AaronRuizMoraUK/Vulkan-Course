#pragma once

#include <RHI/Resource/Image/ImageEnums.h>
#include <RHI/Resource/ResourceEnums.h>

#include <Math/Vector3.h>

namespace Vulkan
{
    struct ImageDesc
    {
        ImageType m_imageType;
        Math::Vector3Int m_dimensions;
        uint32_t m_mipCount = 1; // 1 for no mipmaps. It has be > 0.
        ResourceFormat m_format;

        // When true the native resource is passed in the initial data.
        // When false the initial data is copied into the native texture.
        bool m_initialDataIsNativeResource;
        const void* m_initialData;

        // When true the native resource passed in the initial data is owned
        // by the image and therefore destroyed when Image is destroyed.
        // Not owned by default.
        bool m_ownInitialNativeResource;
    };
} // namespace Vulkan
