#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>

#include <limits>

namespace Vulkan
{
    // Finds the index of the memory type which is in the allowed list and has all the properties passed by argument.
    uint32_t FindCompatibleMemoryTypeIndex(
        VkPhysicalDevice vkPhysicalDevice, uint32_t allowedMemoryTypes, VkMemoryPropertyFlags properties)
    {
        // Get properties of the physical device memory
        VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties = {};
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkPhysicalDeviceMemoryProperties);

        // For each memory type
        for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
        {
            // allowedMemoryTypes is a bit field where each bit is an allowed type,
            // since it's an uint32_t (32 bits) that's 32 different types possible.
            // First one would be the first bit, second would be second bit and so on. 
            const bool allowedMemoryType = (allowedMemoryTypes & (1 << i));

            const bool supportsProperties =
                (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties;

            if (allowedMemoryType && supportsProperties)
            {
                return i;
            }
        }

        DX_LOG(Warning, "Vulkan Utils", "Compatible memory not found!");
        return std::numeric_limits<uint32_t>::max();
    }

    VkFormat ToVkFormat(ResourceFormat format)
    {
        switch (format)
        {
        case ResourceFormat::Unknown:                     return VK_FORMAT_UNDEFINED;

        case ResourceFormat::R8_UNORM:                    return VK_FORMAT_R8_UNORM;
        case ResourceFormat::R8_SNORM:                    return VK_FORMAT_R8_SNORM;
        case ResourceFormat::R8_USCALED:                  return VK_FORMAT_R8_USCALED;
        case ResourceFormat::R8_SSCALED:                  return VK_FORMAT_R8_SSCALED;
        case ResourceFormat::R8_UINT:                     return VK_FORMAT_R8_UINT;
        case ResourceFormat::R8_SINT:                     return VK_FORMAT_R8_SINT;
        case ResourceFormat::R8_SRGB:                     return VK_FORMAT_R8_SRGB;

        case ResourceFormat::R8G8_UNORM:                  return VK_FORMAT_R8G8_UNORM;
        case ResourceFormat::R8G8_SNORM:                  return VK_FORMAT_R8G8_SNORM;
        case ResourceFormat::R8G8_USCALED:                return VK_FORMAT_R8G8_USCALED;
        case ResourceFormat::R8G8_SSCALED:                return VK_FORMAT_R8G8_SSCALED;
        case ResourceFormat::R8G8_UINT:                   return VK_FORMAT_R8G8_UINT;
        case ResourceFormat::R8G8_SINT:                   return VK_FORMAT_R8G8_SINT;
        case ResourceFormat::R8G8_SRGB:                   return VK_FORMAT_R8G8_SRGB;

        case ResourceFormat::R8G8B8_UNORM:                return VK_FORMAT_R8G8B8_UNORM;
        case ResourceFormat::R8G8B8_SNORM:                return VK_FORMAT_R8G8B8_SNORM;
        case ResourceFormat::R8G8B8_USCALED:              return VK_FORMAT_R8G8B8_USCALED;
        case ResourceFormat::R8G8B8_SSCALED:              return VK_FORMAT_R8G8B8_SSCALED;
        case ResourceFormat::R8G8B8_UINT:                 return VK_FORMAT_R8G8B8_UINT;
        case ResourceFormat::R8G8B8_SINT:                 return VK_FORMAT_R8G8B8_SINT;
        case ResourceFormat::R8G8B8_SRGB:                 return VK_FORMAT_R8G8B8_SRGB;

        case ResourceFormat::B8G8R8_UNORM:                return VK_FORMAT_B8G8R8_UNORM;
        case ResourceFormat::B8G8R8_SNORM:                return VK_FORMAT_B8G8R8_SNORM;
        case ResourceFormat::B8G8R8_USCALED:              return VK_FORMAT_B8G8R8_USCALED;
        case ResourceFormat::B8G8R8_SSCALED:              return VK_FORMAT_B8G8R8_SSCALED;
        case ResourceFormat::B8G8R8_UINT:                 return VK_FORMAT_B8G8R8_UINT;
        case ResourceFormat::B8G8R8_SINT:                 return VK_FORMAT_B8G8R8_SINT;
        case ResourceFormat::B8G8R8_SRGB:                 return VK_FORMAT_B8G8R8_SRGB;

        case ResourceFormat::R8G8B8A8_UNORM:              return VK_FORMAT_R8G8B8A8_UNORM;
        case ResourceFormat::R8G8B8A8_SNORM:              return VK_FORMAT_R8G8B8A8_SNORM;
        case ResourceFormat::R8G8B8A8_USCALED:            return VK_FORMAT_R8G8B8A8_USCALED;
        case ResourceFormat::R8G8B8A8_SSCALED:            return VK_FORMAT_R8G8B8A8_SSCALED;
        case ResourceFormat::R8G8B8A8_UINT:               return VK_FORMAT_R8G8B8A8_UINT;
        case ResourceFormat::R8G8B8A8_SINT:               return VK_FORMAT_R8G8B8A8_SINT;
        case ResourceFormat::R8G8B8A8_SRGB:               return VK_FORMAT_R8G8B8A8_SRGB;

        case ResourceFormat::B8G8R8A8_UNORM:              return VK_FORMAT_B8G8R8A8_UNORM;
        case ResourceFormat::B8G8R8A8_SNORM:              return VK_FORMAT_B8G8R8A8_SNORM;
        case ResourceFormat::B8G8R8A8_USCALED:            return VK_FORMAT_B8G8R8A8_USCALED;
        case ResourceFormat::B8G8R8A8_SSCALED:            return VK_FORMAT_B8G8R8A8_SSCALED;
        case ResourceFormat::B8G8R8A8_UINT:               return VK_FORMAT_B8G8R8A8_UINT;
        case ResourceFormat::B8G8R8A8_SINT:               return VK_FORMAT_B8G8R8A8_SINT;
        case ResourceFormat::B8G8R8A8_SRGB:               return VK_FORMAT_B8G8R8A8_SRGB;

        case ResourceFormat::A8B8G8R8_UNORM_PACK32:       return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
        case ResourceFormat::A8B8G8R8_SNORM_PACK32:       return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
        case ResourceFormat::A8B8G8R8_USCALED_PACK32:     return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
        case ResourceFormat::A8B8G8R8_SSCALED_PACK32:     return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
        case ResourceFormat::A8B8G8R8_UINT_PACK32:        return VK_FORMAT_A8B8G8R8_UINT_PACK32;
        case ResourceFormat::A8B8G8R8_SINT_PACK32:        return VK_FORMAT_A8B8G8R8_SINT_PACK32;
        case ResourceFormat::A8B8G8R8_SRGB_PACK32:        return VK_FORMAT_A8B8G8R8_SRGB_PACK32;

        case ResourceFormat::A2R10G10B10_UNORM_PACK32:    return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case ResourceFormat::A2R10G10B10_SNORM_PACK32:    return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
        case ResourceFormat::A2R10G10B10_USCALED_PACK32:  return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
        case ResourceFormat::A2R10G10B10_SSCALED_PACK32:  return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
        case ResourceFormat::A2R10G10B10_UINT_PACK32:     return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case ResourceFormat::A2R10G10B10_SINT_PACK32:     return VK_FORMAT_A2R10G10B10_SINT_PACK32;

        case ResourceFormat::A2B10G10R10_UNORM_PACK32:    return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case ResourceFormat::A2B10G10R10_SNORM_PACK32:    return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
        case ResourceFormat::A2B10G10R10_USCALED_PACK32:  return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
        case ResourceFormat::A2B10G10R10_SSCALED_PACK32:  return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
        case ResourceFormat::A2B10G10R10_UINT_PACK32:     return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case ResourceFormat::A2B10G10R10_SINT_PACK32:     return VK_FORMAT_A2B10G10R10_SINT_PACK32;

        case ResourceFormat::R16_UNORM:                   return VK_FORMAT_R16_UNORM;
        case ResourceFormat::R16_SNORM:                   return VK_FORMAT_R16_SNORM;
        case ResourceFormat::R16_USCALED:                 return VK_FORMAT_R16_USCALED;
        case ResourceFormat::R16_SSCALED:                 return VK_FORMAT_R16_SSCALED;
        case ResourceFormat::R16_UINT:                    return VK_FORMAT_R16_UINT;
        case ResourceFormat::R16_SINT:                    return VK_FORMAT_R16_SINT;
        case ResourceFormat::R16_SFLOAT:                  return VK_FORMAT_R16_SFLOAT;

        case ResourceFormat::R16G16_UNORM:                return VK_FORMAT_R16G16_UNORM;
        case ResourceFormat::R16G16_SNORM:                return VK_FORMAT_R16G16_SNORM;
        case ResourceFormat::R16G16_USCALED:              return VK_FORMAT_R16G16_USCALED;
        case ResourceFormat::R16G16_SSCALED:              return VK_FORMAT_R16G16_SSCALED;
        case ResourceFormat::R16G16_UINT:                 return VK_FORMAT_R16G16_UINT;
        case ResourceFormat::R16G16_SINT:                 return VK_FORMAT_R16G16_SINT;
        case ResourceFormat::R16G16_SFLOAT:               return VK_FORMAT_R16G16_SFLOAT;

        case ResourceFormat::R16G16B16_UNORM:             return VK_FORMAT_R16G16B16_UNORM;
        case ResourceFormat::R16G16B16_SNORM:             return VK_FORMAT_R16G16B16_SNORM;
        case ResourceFormat::R16G16B16_USCALED:           return VK_FORMAT_R16G16B16_USCALED;
        case ResourceFormat::R16G16B16_SSCALED:           return VK_FORMAT_R16G16B16_SSCALED;
        case ResourceFormat::R16G16B16_UINT:              return VK_FORMAT_R16G16B16_UINT;
        case ResourceFormat::R16G16B16_SINT:              return VK_FORMAT_R16G16B16_SINT;
        case ResourceFormat::R16G16B16_SFLOAT:            return VK_FORMAT_R16G16B16_SFLOAT;

        case ResourceFormat::R16G16B16A16_UNORM:          return VK_FORMAT_R16G16B16A16_UNORM;
        case ResourceFormat::R16G16B16A16_SNORM:          return VK_FORMAT_R16G16B16A16_SNORM;
        case ResourceFormat::R16G16B16A16_USCALED:        return VK_FORMAT_R16G16B16A16_USCALED;
        case ResourceFormat::R16G16B16A16_SSCALED:        return VK_FORMAT_R16G16B16A16_SSCALED;
        case ResourceFormat::R16G16B16A16_UINT:           return VK_FORMAT_R16G16B16A16_UINT;
        case ResourceFormat::R16G16B16A16_SINT:           return VK_FORMAT_R16G16B16A16_SINT;
        case ResourceFormat::R16G16B16A16_SFLOAT:         return VK_FORMAT_R16G16B16A16_SFLOAT;

        case ResourceFormat::R32_UINT:                    return VK_FORMAT_R32_UINT;
        case ResourceFormat::R32_SINT:                    return VK_FORMAT_R32_SINT;
        case ResourceFormat::R32_SFLOAT:                  return VK_FORMAT_R32_SFLOAT;
        case ResourceFormat::R32G32_UINT:                 return VK_FORMAT_R32G32_UINT;
        case ResourceFormat::R32G32_SINT:                 return VK_FORMAT_R32G32_SINT;
        case ResourceFormat::R32G32_SFLOAT:               return VK_FORMAT_R32G32_SFLOAT;
        case ResourceFormat::R32G32B32_UINT:              return VK_FORMAT_R32G32B32_UINT;
        case ResourceFormat::R32G32B32_SINT:              return VK_FORMAT_R32G32B32_SINT;
        case ResourceFormat::R32G32B32_SFLOAT:            return VK_FORMAT_R32G32B32_SFLOAT;
        case ResourceFormat::R32G32B32A32_UINT:           return VK_FORMAT_R32G32B32A32_UINT;
        case ResourceFormat::R32G32B32A32_SINT:           return VK_FORMAT_R32G32B32A32_SINT;
        case ResourceFormat::R32G32B32A32_SFLOAT:         return VK_FORMAT_R32G32B32A32_SFLOAT;

        case ResourceFormat::R64_UINT:                    return VK_FORMAT_R64_UINT;
        case ResourceFormat::R64_SINT:                    return VK_FORMAT_R64_SINT;
        case ResourceFormat::R64_SFLOAT:                  return VK_FORMAT_R64_SFLOAT;
        case ResourceFormat::R64G64_UINT:                 return VK_FORMAT_R64G64_UINT;
        case ResourceFormat::R64G64_SINT:                 return VK_FORMAT_R64G64_SINT;
        case ResourceFormat::R64G64_SFLOAT:               return VK_FORMAT_R64G64_SFLOAT;
        case ResourceFormat::R64G64B64_UINT:              return VK_FORMAT_R64G64B64_UINT;
        case ResourceFormat::R64G64B64_SINT:              return VK_FORMAT_R64G64B64_SINT;
        case ResourceFormat::R64G64B64_SFLOAT:            return VK_FORMAT_R64G64B64_SFLOAT;
        case ResourceFormat::R64G64B64A64_UINT:           return VK_FORMAT_R64G64B64A64_UINT;
        case ResourceFormat::R64G64B64A64_SINT:           return VK_FORMAT_R64G64B64A64_SINT;
        case ResourceFormat::R64G64B64A64_SFLOAT:         return VK_FORMAT_R64G64B64A64_SFLOAT;

        case ResourceFormat::B10G11R11_UFLOAT_PACK32:     return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case ResourceFormat::E5B9G9R9_UFLOAT_PACK32:      return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

        case ResourceFormat::D16_UNORM:                   return VK_FORMAT_D16_UNORM;
        case ResourceFormat::X8_D24_UNORM_PACK32:         return VK_FORMAT_X8_D24_UNORM_PACK32;
        case ResourceFormat::D32_SFLOAT:                  return VK_FORMAT_D32_SFLOAT;
        case ResourceFormat::S8_UINT:                     return VK_FORMAT_S8_UINT;
        case ResourceFormat::D16_UNORM_S8_UINT:           return VK_FORMAT_D16_UNORM_S8_UINT;
        case ResourceFormat::D24_UNORM_S8_UINT:           return VK_FORMAT_D24_UNORM_S8_UINT;
        case ResourceFormat::D32_SFLOAT_S8_UINT:          return VK_FORMAT_D32_SFLOAT_S8_UINT;

        case ResourceFormat::BC1_RGB_UNORM_BLOCK:         return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case ResourceFormat::BC1_RGB_SRGB_BLOCK:          return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        case ResourceFormat::BC1_RGBA_UNORM_BLOCK:        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case ResourceFormat::BC1_RGBA_SRGB_BLOCK:         return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case ResourceFormat::BC2_UNORM_BLOCK:             return VK_FORMAT_BC2_UNORM_BLOCK;
        case ResourceFormat::BC2_SRGB_BLOCK:              return VK_FORMAT_BC2_SRGB_BLOCK;
        case ResourceFormat::BC3_UNORM_BLOCK:             return VK_FORMAT_BC3_UNORM_BLOCK;
        case ResourceFormat::BC3_SRGB_BLOCK:              return VK_FORMAT_BC3_SRGB_BLOCK;
        case ResourceFormat::BC4_UNORM_BLOCK:             return VK_FORMAT_BC4_UNORM_BLOCK;
        case ResourceFormat::BC4_SNORM_BLOCK:             return VK_FORMAT_BC4_SNORM_BLOCK;
        case ResourceFormat::BC5_UNORM_BLOCK:             return VK_FORMAT_BC5_UNORM_BLOCK;
        case ResourceFormat::BC5_SNORM_BLOCK:             return VK_FORMAT_BC5_SNORM_BLOCK;
        case ResourceFormat::BC6H_UFLOAT_BLOCK:           return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case ResourceFormat::BC6H_SFLOAT_BLOCK:           return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case ResourceFormat::BC7_UNORM_BLOCK:             return VK_FORMAT_BC7_UNORM_BLOCK;
        case ResourceFormat::BC7_SRGB_BLOCK:              return VK_FORMAT_BC7_SRGB_BLOCK;

        case ResourceFormat::ETC2_R8G8B8_UNORM_BLOCK:     return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case ResourceFormat::ETC2_R8G8B8_SRGB_BLOCK:      return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case ResourceFormat::ETC2_R8G8B8A1_UNORM_BLOCK:   return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case ResourceFormat::ETC2_R8G8B8A1_SRGB_BLOCK:    return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        case ResourceFormat::ETC2_R8G8B8A8_UNORM_BLOCK:   return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case ResourceFormat::ETC2_R8G8B8A8_SRGB_BLOCK:    return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;

        case ResourceFormat::EAC_R11_UNORM_BLOCK:         return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case ResourceFormat::EAC_R11_SNORM_BLOCK:         return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case ResourceFormat::EAC_R11G11_UNORM_BLOCK:      return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case ResourceFormat::EAC_R11G11_SNORM_BLOCK:      return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

        case ResourceFormat::ASTC_4x4_UNORM_BLOCK:        return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case ResourceFormat::ASTC_4x4_SRGB_BLOCK:         return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case ResourceFormat::ASTC_5x4_UNORM_BLOCK:        return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case ResourceFormat::ASTC_5x4_SRGB_BLOCK:         return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case ResourceFormat::ASTC_5x5_UNORM_BLOCK:        return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case ResourceFormat::ASTC_5x5_SRGB_BLOCK:         return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case ResourceFormat::ASTC_6x5_UNORM_BLOCK:        return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case ResourceFormat::ASTC_6x5_SRGB_BLOCK:         return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case ResourceFormat::ASTC_6x6_UNORM_BLOCK:        return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case ResourceFormat::ASTC_6x6_SRGB_BLOCK:         return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case ResourceFormat::ASTC_8x5_UNORM_BLOCK:        return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case ResourceFormat::ASTC_8x5_SRGB_BLOCK:         return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case ResourceFormat::ASTC_8x6_UNORM_BLOCK:        return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case ResourceFormat::ASTC_8x6_SRGB_BLOCK:         return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case ResourceFormat::ASTC_8x8_UNORM_BLOCK:        return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case ResourceFormat::ASTC_8x8_SRGB_BLOCK:         return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case ResourceFormat::ASTC_10x5_UNORM_BLOCK:       return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case ResourceFormat::ASTC_10x5_SRGB_BLOCK:        return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case ResourceFormat::ASTC_10x6_UNORM_BLOCK:       return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case ResourceFormat::ASTC_10x6_SRGB_BLOCK:        return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case ResourceFormat::ASTC_10x8_UNORM_BLOCK:       return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case ResourceFormat::ASTC_10x8_SRGB_BLOCK:        return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case ResourceFormat::ASTC_10x10_UNORM_BLOCK:      return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case ResourceFormat::ASTC_10x10_SRGB_BLOCK:       return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case ResourceFormat::ASTC_12x10_UNORM_BLOCK:      return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case ResourceFormat::ASTC_12x10_SRGB_BLOCK:       return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case ResourceFormat::ASTC_12x12_UNORM_BLOCK:      return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case ResourceFormat::ASTC_12x12_SRGB_BLOCK:       return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

        default:
            DX_LOG(Fatal, "Vulkan Utils", "Unknown resource format %d", format);
            return VK_FORMAT_UNDEFINED;
        }
    }

    ResourceFormat ToResourceFormat(VkFormat vkFormat)
    {
        switch (vkFormat)
        {
        case VK_FORMAT_UNDEFINED:                   return ResourceFormat::Unknown;

        case VK_FORMAT_R8_UNORM:                    return ResourceFormat::R8_UNORM;
        case VK_FORMAT_R8_SNORM:                    return ResourceFormat::R8_SNORM;
        case VK_FORMAT_R8_USCALED:                  return ResourceFormat::R8_USCALED;
        case VK_FORMAT_R8_SSCALED:                  return ResourceFormat::R8_SSCALED;
        case VK_FORMAT_R8_UINT:                     return ResourceFormat::R8_UINT;
        case VK_FORMAT_R8_SINT:                     return ResourceFormat::R8_SINT;
        case VK_FORMAT_R8_SRGB:                     return ResourceFormat::R8_SRGB;

        case VK_FORMAT_R8G8_UNORM:                  return ResourceFormat::R8G8_UNORM;
        case VK_FORMAT_R8G8_SNORM:                  return ResourceFormat::R8G8_SNORM;
        case VK_FORMAT_R8G8_USCALED:                return ResourceFormat::R8G8_USCALED;
        case VK_FORMAT_R8G8_SSCALED:                return ResourceFormat::R8G8_SSCALED;
        case VK_FORMAT_R8G8_UINT:                   return ResourceFormat::R8G8_UINT;
        case VK_FORMAT_R8G8_SINT:                   return ResourceFormat::R8G8_SINT;
        case VK_FORMAT_R8G8_SRGB:                   return ResourceFormat::R8G8_SRGB;

        case VK_FORMAT_R8G8B8_UNORM:                return ResourceFormat::R8G8B8_UNORM;
        case VK_FORMAT_R8G8B8_SNORM:                return ResourceFormat::R8G8B8_SNORM;
        case VK_FORMAT_R8G8B8_USCALED:              return ResourceFormat::R8G8B8_USCALED;
        case VK_FORMAT_R8G8B8_SSCALED:              return ResourceFormat::R8G8B8_SSCALED;
        case VK_FORMAT_R8G8B8_UINT:                 return ResourceFormat::R8G8B8_UINT;
        case VK_FORMAT_R8G8B8_SINT:                 return ResourceFormat::R8G8B8_SINT;
        case VK_FORMAT_R8G8B8_SRGB:                 return ResourceFormat::R8G8B8_SRGB;

        case VK_FORMAT_B8G8R8_UNORM:                return ResourceFormat::B8G8R8_UNORM;
        case VK_FORMAT_B8G8R8_SNORM:                return ResourceFormat::B8G8R8_SNORM;
        case VK_FORMAT_B8G8R8_USCALED:              return ResourceFormat::B8G8R8_USCALED;
        case VK_FORMAT_B8G8R8_SSCALED:              return ResourceFormat::B8G8R8_SSCALED;
        case VK_FORMAT_B8G8R8_UINT:                 return ResourceFormat::B8G8R8_UINT;
        case VK_FORMAT_B8G8R8_SINT:                 return ResourceFormat::B8G8R8_SINT;
        case VK_FORMAT_B8G8R8_SRGB:                 return ResourceFormat::B8G8R8_SRGB;

        case VK_FORMAT_R8G8B8A8_UNORM:              return ResourceFormat::R8G8B8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_SNORM:              return ResourceFormat::R8G8B8A8_SNORM;
        case VK_FORMAT_R8G8B8A8_USCALED:            return ResourceFormat::R8G8B8A8_USCALED;
        case VK_FORMAT_R8G8B8A8_SSCALED:            return ResourceFormat::R8G8B8A8_SSCALED;
        case VK_FORMAT_R8G8B8A8_UINT:               return ResourceFormat::R8G8B8A8_UINT;
        case VK_FORMAT_R8G8B8A8_SINT:               return ResourceFormat::R8G8B8A8_SINT;
        case VK_FORMAT_R8G8B8A8_SRGB:               return ResourceFormat::R8G8B8A8_SRGB;

        case VK_FORMAT_B8G8R8A8_UNORM:              return ResourceFormat::B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SNORM:              return ResourceFormat::B8G8R8A8_SNORM;
        case VK_FORMAT_B8G8R8A8_USCALED:            return ResourceFormat::B8G8R8A8_USCALED;
        case VK_FORMAT_B8G8R8A8_SSCALED:            return ResourceFormat::B8G8R8A8_SSCALED;
        case VK_FORMAT_B8G8R8A8_UINT:               return ResourceFormat::B8G8R8A8_UINT;
        case VK_FORMAT_B8G8R8A8_SINT:               return ResourceFormat::B8G8R8A8_SINT;
        case VK_FORMAT_B8G8R8A8_SRGB:               return ResourceFormat::B8G8R8A8_SRGB;

        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:       return ResourceFormat::A8B8G8R8_UNORM_PACK32;
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:       return ResourceFormat::A8B8G8R8_SNORM_PACK32;
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:     return ResourceFormat::A8B8G8R8_USCALED_PACK32;
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:     return ResourceFormat::A8B8G8R8_SSCALED_PACK32;
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:        return ResourceFormat::A8B8G8R8_UINT_PACK32;
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:        return ResourceFormat::A8B8G8R8_SINT_PACK32;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:        return ResourceFormat::A8B8G8R8_SRGB_PACK32;

        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:    return ResourceFormat::A2R10G10B10_UNORM_PACK32;
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:    return ResourceFormat::A2R10G10B10_SNORM_PACK32;
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:  return ResourceFormat::A2R10G10B10_USCALED_PACK32;
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:  return ResourceFormat::A2R10G10B10_SSCALED_PACK32;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:     return ResourceFormat::A2R10G10B10_UINT_PACK32;
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:     return ResourceFormat::A2R10G10B10_SINT_PACK32;

        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:    return ResourceFormat::A2B10G10R10_UNORM_PACK32;
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:    return ResourceFormat::A2B10G10R10_SNORM_PACK32;
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:  return ResourceFormat::A2B10G10R10_USCALED_PACK32;
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:  return ResourceFormat::A2B10G10R10_SSCALED_PACK32;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:     return ResourceFormat::A2B10G10R10_UINT_PACK32;
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:     return ResourceFormat::A2B10G10R10_SINT_PACK32;

        case VK_FORMAT_R16_UNORM:                   return ResourceFormat::R16_UNORM;
        case VK_FORMAT_R16_SNORM:                   return ResourceFormat::R16_SNORM;
        case VK_FORMAT_R16_USCALED:                 return ResourceFormat::R16_USCALED;
        case VK_FORMAT_R16_SSCALED:                 return ResourceFormat::R16_SSCALED;
        case VK_FORMAT_R16_UINT:                    return ResourceFormat::R16_UINT;
        case VK_FORMAT_R16_SINT:                    return ResourceFormat::R16_SINT;
        case VK_FORMAT_R16_SFLOAT:                  return ResourceFormat::R16_SFLOAT;

        case VK_FORMAT_R16G16_UNORM:                return ResourceFormat::R16G16_UNORM;
        case VK_FORMAT_R16G16_SNORM:                return ResourceFormat::R16G16_SNORM;
        case VK_FORMAT_R16G16_USCALED:              return ResourceFormat::R16G16_USCALED;
        case VK_FORMAT_R16G16_SSCALED:              return ResourceFormat::R16G16_SSCALED;
        case VK_FORMAT_R16G16_UINT:                 return ResourceFormat::R16G16_UINT;
        case VK_FORMAT_R16G16_SINT:                 return ResourceFormat::R16G16_SINT;
        case VK_FORMAT_R16G16_SFLOAT:               return ResourceFormat::R16G16_SFLOAT;

        case VK_FORMAT_R16G16B16_UNORM:             return ResourceFormat::R16G16B16_UNORM;
        case VK_FORMAT_R16G16B16_SNORM:             return ResourceFormat::R16G16B16_SNORM;
        case VK_FORMAT_R16G16B16_USCALED:           return ResourceFormat::R16G16B16_USCALED;
        case VK_FORMAT_R16G16B16_SSCALED:           return ResourceFormat::R16G16B16_SSCALED;
        case VK_FORMAT_R16G16B16_UINT:              return ResourceFormat::R16G16B16_UINT;
        case VK_FORMAT_R16G16B16_SINT:              return ResourceFormat::R16G16B16_SINT;
        case VK_FORMAT_R16G16B16_SFLOAT:            return ResourceFormat::R16G16B16_SFLOAT;

        case VK_FORMAT_R16G16B16A16_UNORM:          return ResourceFormat::R16G16B16A16_UNORM;
        case VK_FORMAT_R16G16B16A16_SNORM:          return ResourceFormat::R16G16B16A16_SNORM;
        case VK_FORMAT_R16G16B16A16_USCALED:        return ResourceFormat::R16G16B16A16_USCALED;
        case VK_FORMAT_R16G16B16A16_SSCALED:        return ResourceFormat::R16G16B16A16_SSCALED;
        case VK_FORMAT_R16G16B16A16_UINT:           return ResourceFormat::R16G16B16A16_UINT;
        case VK_FORMAT_R16G16B16A16_SINT:           return ResourceFormat::R16G16B16A16_SINT;
        case VK_FORMAT_R16G16B16A16_SFLOAT:         return ResourceFormat::R16G16B16A16_SFLOAT;

        case VK_FORMAT_R32_UINT:                    return ResourceFormat::R32_UINT;
        case VK_FORMAT_R32_SINT:                    return ResourceFormat::R32_SINT;
        case VK_FORMAT_R32_SFLOAT:                  return ResourceFormat::R32_SFLOAT;
        case VK_FORMAT_R32G32_UINT:                 return ResourceFormat::R32G32_UINT;
        case VK_FORMAT_R32G32_SINT:                 return ResourceFormat::R32G32_SINT;
        case VK_FORMAT_R32G32_SFLOAT:               return ResourceFormat::R32G32_SFLOAT;
        case VK_FORMAT_R32G32B32_UINT:              return ResourceFormat::R32G32B32_UINT;
        case VK_FORMAT_R32G32B32_SINT:              return ResourceFormat::R32G32B32_SINT;
        case VK_FORMAT_R32G32B32_SFLOAT:            return ResourceFormat::R32G32B32_SFLOAT;
        case VK_FORMAT_R32G32B32A32_UINT:           return ResourceFormat::R32G32B32A32_UINT;
        case VK_FORMAT_R32G32B32A32_SINT:           return ResourceFormat::R32G32B32A32_SINT;
        case VK_FORMAT_R32G32B32A32_SFLOAT:         return ResourceFormat::R32G32B32A32_SFLOAT;

        case VK_FORMAT_R64_UINT:                    return ResourceFormat::R64_UINT;
        case VK_FORMAT_R64_SINT:                    return ResourceFormat::R64_SINT;
        case VK_FORMAT_R64_SFLOAT:                  return ResourceFormat::R64_SFLOAT;
        case VK_FORMAT_R64G64_UINT:                 return ResourceFormat::R64G64_UINT;
        case VK_FORMAT_R64G64_SINT:                 return ResourceFormat::R64G64_SINT;
        case VK_FORMAT_R64G64_SFLOAT:               return ResourceFormat::R64G64_SFLOAT;
        case VK_FORMAT_R64G64B64_UINT:              return ResourceFormat::R64G64B64_UINT;
        case VK_FORMAT_R64G64B64_SINT:              return ResourceFormat::R64G64B64_SINT;
        case VK_FORMAT_R64G64B64_SFLOAT:            return ResourceFormat::R64G64B64_SFLOAT;
        case VK_FORMAT_R64G64B64A64_UINT:           return ResourceFormat::R64G64B64A64_UINT;
        case VK_FORMAT_R64G64B64A64_SINT:           return ResourceFormat::R64G64B64A64_SINT;
        case VK_FORMAT_R64G64B64A64_SFLOAT:         return ResourceFormat::R64G64B64A64_SFLOAT;

        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:     return ResourceFormat::B10G11R11_UFLOAT_PACK32;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:      return ResourceFormat::E5B9G9R9_UFLOAT_PACK32;

        case VK_FORMAT_D16_UNORM:                   return ResourceFormat::D16_UNORM;
        case VK_FORMAT_X8_D24_UNORM_PACK32:         return ResourceFormat::X8_D24_UNORM_PACK32;
        case VK_FORMAT_D32_SFLOAT:                  return ResourceFormat::D32_SFLOAT;
        case VK_FORMAT_S8_UINT:                     return ResourceFormat::S8_UINT;
        case VK_FORMAT_D16_UNORM_S8_UINT:           return ResourceFormat::D16_UNORM_S8_UINT;
        case VK_FORMAT_D24_UNORM_S8_UINT:           return ResourceFormat::D24_UNORM_S8_UINT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:          return ResourceFormat::D32_SFLOAT_S8_UINT;

        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:         return ResourceFormat::BC1_RGB_UNORM_BLOCK;
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:          return ResourceFormat::BC1_RGB_SRGB_BLOCK;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:        return ResourceFormat::BC1_RGBA_UNORM_BLOCK;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:         return ResourceFormat::BC1_RGBA_SRGB_BLOCK;
        case VK_FORMAT_BC2_UNORM_BLOCK:             return ResourceFormat::BC2_UNORM_BLOCK;
        case VK_FORMAT_BC2_SRGB_BLOCK:              return ResourceFormat::BC2_SRGB_BLOCK;
        case VK_FORMAT_BC3_UNORM_BLOCK:             return ResourceFormat::BC3_UNORM_BLOCK;
        case VK_FORMAT_BC3_SRGB_BLOCK:              return ResourceFormat::BC3_SRGB_BLOCK;
        case VK_FORMAT_BC4_UNORM_BLOCK:             return ResourceFormat::BC4_UNORM_BLOCK;
        case VK_FORMAT_BC4_SNORM_BLOCK:             return ResourceFormat::BC4_SNORM_BLOCK;
        case VK_FORMAT_BC5_UNORM_BLOCK:             return ResourceFormat::BC5_UNORM_BLOCK;
        case VK_FORMAT_BC5_SNORM_BLOCK:             return ResourceFormat::BC5_SNORM_BLOCK;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:           return ResourceFormat::BC6H_UFLOAT_BLOCK;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:           return ResourceFormat::BC6H_SFLOAT_BLOCK;
        case VK_FORMAT_BC7_UNORM_BLOCK:             return ResourceFormat::BC7_UNORM_BLOCK;
        case VK_FORMAT_BC7_SRGB_BLOCK:              return ResourceFormat::BC7_SRGB_BLOCK;

        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:     return ResourceFormat::ETC2_R8G8B8_UNORM_BLOCK;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:      return ResourceFormat::ETC2_R8G8B8_SRGB_BLOCK;
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:   return ResourceFormat::ETC2_R8G8B8A1_UNORM_BLOCK;
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:    return ResourceFormat::ETC2_R8G8B8A1_SRGB_BLOCK;
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:   return ResourceFormat::ETC2_R8G8B8A8_UNORM_BLOCK;
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:    return ResourceFormat::ETC2_R8G8B8A8_SRGB_BLOCK;

        case VK_FORMAT_EAC_R11_UNORM_BLOCK:         return ResourceFormat::EAC_R11_UNORM_BLOCK;
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:         return ResourceFormat::EAC_R11_SNORM_BLOCK;
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:      return ResourceFormat::EAC_R11G11_UNORM_BLOCK;
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:      return ResourceFormat::EAC_R11G11_SNORM_BLOCK;

        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:        return ResourceFormat::ASTC_4x4_UNORM_BLOCK;
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:         return ResourceFormat::ASTC_4x4_SRGB_BLOCK;
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:        return ResourceFormat::ASTC_5x4_UNORM_BLOCK;
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:         return ResourceFormat::ASTC_5x4_SRGB_BLOCK;
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:        return ResourceFormat::ASTC_5x5_UNORM_BLOCK;
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:         return ResourceFormat::ASTC_5x5_SRGB_BLOCK;
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:        return ResourceFormat::ASTC_6x5_UNORM_BLOCK;
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:         return ResourceFormat::ASTC_6x5_SRGB_BLOCK;
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:        return ResourceFormat::ASTC_6x6_UNORM_BLOCK;
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:         return ResourceFormat::ASTC_6x6_SRGB_BLOCK;
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:        return ResourceFormat::ASTC_8x5_UNORM_BLOCK;
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:         return ResourceFormat::ASTC_8x5_SRGB_BLOCK;
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:        return ResourceFormat::ASTC_8x6_UNORM_BLOCK;
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:         return ResourceFormat::ASTC_8x6_SRGB_BLOCK;
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:        return ResourceFormat::ASTC_8x8_UNORM_BLOCK;
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:         return ResourceFormat::ASTC_8x8_SRGB_BLOCK;
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:       return ResourceFormat::ASTC_10x5_UNORM_BLOCK;
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:        return ResourceFormat::ASTC_10x5_SRGB_BLOCK;
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:       return ResourceFormat::ASTC_10x6_UNORM_BLOCK;
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:        return ResourceFormat::ASTC_10x6_SRGB_BLOCK;
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:       return ResourceFormat::ASTC_10x8_UNORM_BLOCK;
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:        return ResourceFormat::ASTC_10x8_SRGB_BLOCK;
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:      return ResourceFormat::ASTC_10x10_UNORM_BLOCK;
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:       return ResourceFormat::ASTC_10x10_SRGB_BLOCK;
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:      return ResourceFormat::ASTC_12x10_UNORM_BLOCK;
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:       return ResourceFormat::ASTC_12x10_SRGB_BLOCK;
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:      return ResourceFormat::ASTC_12x12_UNORM_BLOCK;
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:       return ResourceFormat::ASTC_12x12_SRGB_BLOCK;

        default:
            DX_LOG(Fatal, "Vulkan Utils", "Unknown resource format %d", vkFormat);
            return ResourceFormat::Unknown;
        }
    }

    VkImageType ToVkImageType(ImageType imageType)
    {
        switch (imageType)
        {
        case ImageType::Image1D: return VK_IMAGE_TYPE_1D;
        case ImageType::Image2D: return VK_IMAGE_TYPE_2D;
        case ImageType::Image3D: return VK_IMAGE_TYPE_3D;

        case ImageType::Unknown:
        default:
            DX_LOG(Fatal, "Vulkan Utils", "Unknown image type %d", imageType);
            return VK_IMAGE_TYPE_MAX_ENUM;
        }
    }

    VkImageViewType ToVkImageViewType(ImageType imageType)
    {
        switch (imageType)
        {
        case ImageType::Image1D: return VK_IMAGE_VIEW_TYPE_1D;
        case ImageType::Image2D: return VK_IMAGE_VIEW_TYPE_2D;
        case ImageType::Image3D: return VK_IMAGE_VIEW_TYPE_3D;

        case ImageType::Unknown:
        default:
            DX_LOG(Fatal, "Vulkan Utils", "Unknown image type %d", imageType);
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        }
    }

    VkImageTiling ToVkImageTiling(ImageTiling imageTiling)
    {
        switch (imageTiling)
        {
        case ImageTiling::Optimal: return VK_IMAGE_TILING_OPTIMAL;
        case ImageTiling::Linear: return VK_IMAGE_TILING_LINEAR;

        case ImageTiling::Unknown:
        default:
            DX_LOG(Fatal, "Vulkan Utils", "Unknown image tiling %d", imageTiling);
            return VK_IMAGE_TILING_MAX_ENUM;
        }
    }

    VkImageUsageFlags ToVkImageUsageFlags(ImageUsageFlags flags)
    {
        VkImageUsageFlags vkImageUsageFlags = 0;

        vkImageUsageFlags |= (flags & ImageUsage_Sampled) ? VK_IMAGE_USAGE_SAMPLED_BIT : 0;
        vkImageUsageFlags |= (flags & ImageUsage_Storage) ? VK_IMAGE_USAGE_STORAGE_BIT : 0;
        vkImageUsageFlags |= (flags & ImageUsage_ColorAttachment) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
        vkImageUsageFlags |= (flags & ImageUsage_DepthStencilAttachment) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0;
        vkImageUsageFlags |= (flags & ImageUsage_InputAttachment) ? VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT : 0;

        return vkImageUsageFlags;
    }

    VkImageAspectFlags ToVkImageAspectFlags(ImageViewAspectFlags flags)
    {
        VkImageAspectFlags vkImageAspectFlags = 0;

        vkImageAspectFlags |= (flags & ImageViewAspect_Color) ? VK_IMAGE_ASPECT_COLOR_BIT : 0;
        vkImageAspectFlags |= (flags & ImageViewAspect_Depth) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
        vkImageAspectFlags |= (flags & ImageViewAspect_Stencil) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;

        return vkImageAspectFlags;
    }

    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags)
    {
        VkBufferUsageFlags vkBufferUsageFlags = 0;

        vkBufferUsageFlags |= (flags & BufferUsage_VertexBuffer) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
        vkBufferUsageFlags |= (flags & BufferUsage_IndexBuffer) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
        vkBufferUsageFlags |= (flags & BufferUsage_UniformBuffer) ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;
        vkBufferUsageFlags |= (flags & BufferUsage_TransferSrc) ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT: 0;
        vkBufferUsageFlags |= (flags & BufferUsage_TransferDst) ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0;

        return vkBufferUsageFlags;
    }

    VkFilter ToVkFilter(FilterSampling filter)
    {
        switch (filter)
        {
        case FilterSampling::Point:         return VK_FILTER_NEAREST;
        case FilterSampling::Linear:        return VK_FILTER_LINEAR;
        case FilterSampling::Anisotropic:   return VK_FILTER_LINEAR; // Anisotropic gets enabled in the sampler separately

        case FilterSampling::Unknown:
        default:
            DX_LOG(Error, "Vulkan Utils", "Unknown filter sampling %d", filter);
            return VK_FILTER_NEAREST;
        }
    }

    VkSamplerMipmapMode ToVkSamplerMipmapMode(FilterSampling filter)
    {
        switch (filter)
        {
        case FilterSampling::Point:         return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case FilterSampling::Linear:        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case FilterSampling::Anisotropic:   return VK_SAMPLER_MIPMAP_MODE_LINEAR; // Anisotropic gets enabled in the sampler separately

        case FilterSampling::Unknown:
        default:
            DX_LOG(Error, "Vulkan Utils", "Unknown filter sampling %d", filter);
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
    }

    VkSamplerAddressMode ToVkSamplerAddressMode(AddressMode addressMode)
    {
        switch (addressMode)
        {
        case AddressMode::Wrap:        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case AddressMode::Mirror:      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case AddressMode::Clamp:       return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case AddressMode::MirrorOnce:  return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

        case AddressMode::Unknown:
        default:
            DX_LOG(Error, "Vulkan Utils", "Unknown address mode %d", addressMode);
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags)
    {
        VkCommandBufferUsageFlags vkCommandBufferUsageFlags = 0;

        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_OneTimeSubmit) ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_RenderPassContinue) ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0;
        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_SimultaneousUse) ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0;

        return vkCommandBufferUsageFlags;
    }

    VkShaderStageFlags ToVkShaderStageFlags(ShaderTypeFlags flags)
    {
        VkShaderStageFlags vkShaderStageFlags = 0;

        vkShaderStageFlags |= (flags & ShaderType_Vertex) ? VK_SHADER_STAGE_VERTEX_BIT : 0;
        vkShaderStageFlags |= (flags & ShaderType_TesselationControl) ? VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT : 0;
        vkShaderStageFlags |= (flags & ShaderType_TesselationEvaluation) ? VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT : 0;
        vkShaderStageFlags |= (flags & ShaderType_Geometry) ? VK_SHADER_STAGE_GEOMETRY_BIT : 0;
        vkShaderStageFlags |= (flags & ShaderType_Fragment) ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
        vkShaderStageFlags |= (flags & ShaderType_Compute) ? VK_SHADER_STAGE_COMPUTE_BIT : 0;

        return vkShaderStageFlags;
    }

} // namespace Vulkan
