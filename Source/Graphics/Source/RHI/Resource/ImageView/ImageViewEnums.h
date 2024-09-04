#pragma once

#include <stdint.h>

namespace Vulkan
{
    // Bitwise operations on ImageUsageFlag are allowed.
    enum ImageViewAspectFlag
    {
        ImageViewAspect_Color = 1 << 0,
        ImageViewAspect_Depth = 1 << 1,
        ImageViewAspect_Stencil = 1 << 2
    };
    using ImageViewAspectFlags = uint32_t;

} // namespace Vulkan
