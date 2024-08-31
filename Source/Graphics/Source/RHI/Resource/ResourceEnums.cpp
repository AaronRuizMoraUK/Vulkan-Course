#include <RHI/Resource/ResourceEnums.h>

#include <Log/Log.h>

namespace Vulkan
{
    int ResourceFormatSize(ResourceFormat format, int elementCount)
    {
        switch (format)
        {
        case ResourceFormat::R8_UNORM:                    return elementCount * 1;
        case ResourceFormat::R8_SNORM:                    return elementCount * 1;
        case ResourceFormat::R8_USCALED:                  return elementCount * 1;
        case ResourceFormat::R8_SSCALED:                  return elementCount * 1;
        case ResourceFormat::R8_UINT:                     return elementCount * 1;
        case ResourceFormat::R8_SINT:                     return elementCount * 1;
        case ResourceFormat::R8_SRGB:                     return elementCount * 1;

        case ResourceFormat::R8G8_UNORM:                  return elementCount * 2;
        case ResourceFormat::R8G8_SNORM:                  return elementCount * 2;
        case ResourceFormat::R8G8_USCALED:                return elementCount * 2;
        case ResourceFormat::R8G8_SSCALED:                return elementCount * 2;
        case ResourceFormat::R8G8_UINT:                   return elementCount * 2;
        case ResourceFormat::R8G8_SINT:                   return elementCount * 2;
        case ResourceFormat::R8G8_SRGB:                   return elementCount * 2;

        case ResourceFormat::R8G8B8_UNORM:                return elementCount * 3;
        case ResourceFormat::R8G8B8_SNORM:                return elementCount * 3;
        case ResourceFormat::R8G8B8_USCALED:              return elementCount * 3;
        case ResourceFormat::R8G8B8_SSCALED:              return elementCount * 3;
        case ResourceFormat::R8G8B8_UINT:                 return elementCount * 3;
        case ResourceFormat::R8G8B8_SINT:                 return elementCount * 3;
        case ResourceFormat::R8G8B8_SRGB:                 return elementCount * 3;

        case ResourceFormat::B8G8R8_UNORM:                return elementCount * 3;
        case ResourceFormat::B8G8R8_SNORM:                return elementCount * 3;
        case ResourceFormat::B8G8R8_USCALED:              return elementCount * 3;
        case ResourceFormat::B8G8R8_SSCALED:              return elementCount * 3;
        case ResourceFormat::B8G8R8_UINT:                 return elementCount * 3;
        case ResourceFormat::B8G8R8_SINT:                 return elementCount * 3;
        case ResourceFormat::B8G8R8_SRGB:                 return elementCount * 3;

        case ResourceFormat::R8G8B8A8_UNORM:              return elementCount * 4;
        case ResourceFormat::R8G8B8A8_SNORM:              return elementCount * 4;
        case ResourceFormat::R8G8B8A8_USCALED:            return elementCount * 4;
        case ResourceFormat::R8G8B8A8_SSCALED:            return elementCount * 4;
        case ResourceFormat::R8G8B8A8_UINT:               return elementCount * 4;
        case ResourceFormat::R8G8B8A8_SINT:               return elementCount * 4;
        case ResourceFormat::R8G8B8A8_SRGB:               return elementCount * 4;

        case ResourceFormat::B8G8R8A8_UNORM:              return elementCount * 4;
        case ResourceFormat::B8G8R8A8_SNORM:              return elementCount * 4;
        case ResourceFormat::B8G8R8A8_USCALED:            return elementCount * 4;
        case ResourceFormat::B8G8R8A8_SSCALED:            return elementCount * 4;
        case ResourceFormat::B8G8R8A8_UINT:               return elementCount * 4;
        case ResourceFormat::B8G8R8A8_SINT:               return elementCount * 4;
        case ResourceFormat::B8G8R8A8_SRGB:               return elementCount * 4;

        case ResourceFormat::A8B8G8R8_UNORM_PACK32:       return elementCount * 4;
        case ResourceFormat::A8B8G8R8_SNORM_PACK32:       return elementCount * 4;
        case ResourceFormat::A8B8G8R8_USCALED_PACK32:     return elementCount * 4;
        case ResourceFormat::A8B8G8R8_SSCALED_PACK32:     return elementCount * 4;
        case ResourceFormat::A8B8G8R8_UINT_PACK32:        return elementCount * 4;
        case ResourceFormat::A8B8G8R8_SINT_PACK32:        return elementCount * 4;
        case ResourceFormat::A8B8G8R8_SRGB_PACK32:        return elementCount * 4;

        case ResourceFormat::A2R10G10B10_UNORM_PACK32:    return elementCount * 4;
        case ResourceFormat::A2R10G10B10_SNORM_PACK32:    return elementCount * 4;
        case ResourceFormat::A2R10G10B10_USCALED_PACK32:  return elementCount * 4;
        case ResourceFormat::A2R10G10B10_SSCALED_PACK32:  return elementCount * 4;
        case ResourceFormat::A2R10G10B10_UINT_PACK32:     return elementCount * 4;
        case ResourceFormat::A2R10G10B10_SINT_PACK32:     return elementCount * 4;

        case ResourceFormat::A2B10G10R10_UNORM_PACK32:    return elementCount * 4;
        case ResourceFormat::A2B10G10R10_SNORM_PACK32:    return elementCount * 4;
        case ResourceFormat::A2B10G10R10_USCALED_PACK32:  return elementCount * 4;
        case ResourceFormat::A2B10G10R10_SSCALED_PACK32:  return elementCount * 4;
        case ResourceFormat::A2B10G10R10_UINT_PACK32:     return elementCount * 4;
        case ResourceFormat::A2B10G10R10_SINT_PACK32:     return elementCount * 4;

        case ResourceFormat::R16_UNORM:                   return elementCount * 2;
        case ResourceFormat::R16_SNORM:                   return elementCount * 2;
        case ResourceFormat::R16_USCALED:                 return elementCount * 2;
        case ResourceFormat::R16_SSCALED:                 return elementCount * 2;
        case ResourceFormat::R16_UINT:                    return elementCount * 2;
        case ResourceFormat::R16_SINT:                    return elementCount * 2;
        case ResourceFormat::R16_SFLOAT:                  return elementCount * 2;

        case ResourceFormat::R16G16_UNORM:                return elementCount * 4;
        case ResourceFormat::R16G16_SNORM:                return elementCount * 4;
        case ResourceFormat::R16G16_USCALED:              return elementCount * 4;
        case ResourceFormat::R16G16_SSCALED:              return elementCount * 4;
        case ResourceFormat::R16G16_UINT:                 return elementCount * 4;
        case ResourceFormat::R16G16_SINT:                 return elementCount * 4;
        case ResourceFormat::R16G16_SFLOAT:               return elementCount * 4;

        case ResourceFormat::R16G16B16_UNORM:             return elementCount * 6;
        case ResourceFormat::R16G16B16_SNORM:             return elementCount * 6;
        case ResourceFormat::R16G16B16_USCALED:           return elementCount * 6;
        case ResourceFormat::R16G16B16_SSCALED:           return elementCount * 6;
        case ResourceFormat::R16G16B16_UINT:              return elementCount * 6;
        case ResourceFormat::R16G16B16_SINT:              return elementCount * 6;
        case ResourceFormat::R16G16B16_SFLOAT:            return elementCount * 6;

        case ResourceFormat::R16G16B16A16_UNORM:          return elementCount * 8;
        case ResourceFormat::R16G16B16A16_SNORM:          return elementCount * 8;
        case ResourceFormat::R16G16B16A16_USCALED:        return elementCount * 8;
        case ResourceFormat::R16G16B16A16_SSCALED:        return elementCount * 8;
        case ResourceFormat::R16G16B16A16_UINT:           return elementCount * 8;
        case ResourceFormat::R16G16B16A16_SINT:           return elementCount * 8;
        case ResourceFormat::R16G16B16A16_SFLOAT:         return elementCount * 8;

        case ResourceFormat::R32_UINT:                    return elementCount * 4;
        case ResourceFormat::R32_SINT:                    return elementCount * 4;
        case ResourceFormat::R32_SFLOAT:                  return elementCount * 4;

        case ResourceFormat::R32G32_UINT:                 return elementCount * 8;
        case ResourceFormat::R32G32_SINT:                 return elementCount * 8;
        case ResourceFormat::R32G32_SFLOAT:               return elementCount * 8;

        case ResourceFormat::R32G32B32_UINT:              return elementCount * 12;
        case ResourceFormat::R32G32B32_SINT:              return elementCount * 12;
        case ResourceFormat::R32G32B32_SFLOAT:            return elementCount * 12;

        case ResourceFormat::R32G32B32A32_UINT:           return elementCount * 16;
        case ResourceFormat::R32G32B32A32_SINT:           return elementCount * 16;
        case ResourceFormat::R32G32B32A32_SFLOAT:         return elementCount * 16;

        case ResourceFormat::R64_UINT:                    return elementCount * 8;
        case ResourceFormat::R64_SINT:                    return elementCount * 8;
        case ResourceFormat::R64_SFLOAT:                  return elementCount * 8;

        case ResourceFormat::R64G64_UINT:                 return elementCount * 16;
        case ResourceFormat::R64G64_SINT:                 return elementCount * 16;
        case ResourceFormat::R64G64_SFLOAT:               return elementCount * 16;

        case ResourceFormat::R64G64B64_UINT:              return elementCount * 24;
        case ResourceFormat::R64G64B64_SINT:              return elementCount * 24;
        case ResourceFormat::R64G64B64_SFLOAT:            return elementCount * 24;

        case ResourceFormat::R64G64B64A64_UINT:           return elementCount * 32;
        case ResourceFormat::R64G64B64A64_SINT:           return elementCount * 32;
        case ResourceFormat::R64G64B64A64_SFLOAT:         return elementCount * 32;

        case ResourceFormat::B10G11R11_UFLOAT_PACK32:     return elementCount * 4;
        case ResourceFormat::E5B9G9R9_UFLOAT_PACK32:      return elementCount * 4;

        case ResourceFormat::D16_UNORM:                   return elementCount * 2;
        case ResourceFormat::X8_D24_UNORM_PACK32:         return elementCount * 4;
        case ResourceFormat::D32_SFLOAT:                  return elementCount * 4;
        case ResourceFormat::S8_UINT:                     return elementCount * 1;
        case ResourceFormat::D16_UNORM_S8_UINT:           return elementCount * 3;
        case ResourceFormat::D24_UNORM_S8_UINT:           return elementCount * 4;
        case ResourceFormat::D32_SFLOAT_S8_UINT:          return elementCount * 5;

        default:
            DX_LOG(Fatal, "ResourceFormat", "Unknown size for resource format %d", format);
            return 0;
        }
    }
} // namespace Vulkan
