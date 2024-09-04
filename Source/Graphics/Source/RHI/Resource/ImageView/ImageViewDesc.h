#pragma once

#include <RHI/Resource/ResourceEnums.h>
#include <RHI/Resource/ImageView/ImageViewEnums.h>

#include <memory>

namespace Vulkan
{
    class Image;

    struct ImageViewDesc
    {
        std::shared_ptr<Image> m_image;
        ResourceFormat m_viewFormat;
        ImageViewAspectFlags m_aspectFlags; // Bitwise operation of ImageViewAspectFlags

        uint32_t m_firstMip; // Index of the first mipmap level to use.
        uint32_t m_mipCount; // Number of mipmap levels to use, starting from m_firstMip. Use 0 for all mipmaps starting from m_firstMip.
    };
} // namespace Vulkan
