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
} // namespace Vulkan
