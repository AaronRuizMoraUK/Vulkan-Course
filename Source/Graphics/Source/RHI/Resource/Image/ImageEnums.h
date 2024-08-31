#pragma once

#include <stdint.h>

namespace Vulkan
{
    enum class ImageType
    {
        Unknown = 0,

        Image1D,
        Image2D,
        Image3D,

        Count
    };

    const char* ImageTypeStr(ImageType imageType);

    // Bitwise operations on ImageUsageFlag are allowed.
    enum ImageUsageFlag
    {
        ImageUsage_Sampled = 1 << 0,
        ImageUsage_Storage = 1 << 1,
        ImageUsage_ColorAttachment = 1 << 2,
        ImageUsage_DepthStencilAttachment = 1 << 3
    };
    using ImageUsageFlags = uint32_t;

    // How memory is arranged for optimal reading
    enum class ImageTiling
    {
        Unknown = 0,

        Optimal,
        Linear,

        Count
    };
} // namespace Vulkan
