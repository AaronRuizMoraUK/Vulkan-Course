#pragma once

#include <RHI/Sampler/SamplerEnums.h>

namespace Vulkan
{
    struct SamplerDesc
    {
        // NOTE: If any filter is set to Anisotropic then all of them will be set to anisotropic.
        FilterSampling m_minFilter; // Minification: Filter to apply when texture is bigger than its space in screen
        FilterSampling m_magFilter; // Magnification: Filter to apply when texture is smaller than its space in screen
        FilterSampling m_mipFilter; // Filter to apply between mipmaps

        AddressMode m_addressU;
        AddressMode m_addressV;
        AddressMode m_addressW;

        float m_mipBias;
        Math::Vector2 m_mipClamp; // 0.0f is the largest mipmap. For no clamping use Vulkan::NoMipClamping.

        float m_maxAnisotropy; // Valid values are between 1.0f and VkPhysicalDeviceLimits::maxSamplerAnisotropy
    };
} // namespace Vulkan
