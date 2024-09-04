#pragma once

#include <Math/Vector2.h>

namespace Vulkan
{
    enum class FilterSampling
    {
        Unknown = 0,

        Point,
        Linear,
        Anisotropic, // Requires to enable Anisotropy feature in device

        Count
    };

    // How to sample with texture coordinate outside [0,1]
    enum class AddressMode
    {
        Unknown = 0,

        Wrap,
        Mirror,
        Clamp,
        MirrorOnce,

        Count
    };

    static const float MaxMipLevel = 1000.0f; // Same as VK_LOD_CLAMP_NONE
    static const Math::Vector2 NoMipClamping = Math::Vector2(0.0f, MaxMipLevel);
} // namespace Vulkan
