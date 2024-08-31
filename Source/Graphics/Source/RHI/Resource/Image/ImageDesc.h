#pragma once

#include <RHI/Resource/Image/ImageEnums.h>
#include <RHI/Resource/ResourceEnums.h>

#include <Math/Vector3.h>

#include <optional>

namespace Vulkan
{
    struct ImageDesc
    {
        ImageType m_imageType;
        Math::Vector3Int m_dimensions;
        uint32_t m_mipCount = 1; // 1 for no mipmaps. It has be > 0.
        ResourceFormat m_format;
        ImageTiling m_tiling;
        ImageUsageFlags m_usageFlags; // Bitwise operation of ImageUsageFlags
        ResourceMemoryProperty m_memoryProperty;

        const void* m_initialData;

        // When native resources are passed it will directly use them 
        // and not create them.
        struct NativeResource
        {
            void* m_imageNativeResource;
            void* m_imageMemoryNativeResource;

            // When true, the native resources are owned by the image and
            // therefore will be destroyed when the Image is destroyed.
            // Not owned by default.
            bool m_ownsNativeResource;
        };
        std::optional<NativeResource> m_nativeResource;
    };
} // namespace Vulkan
