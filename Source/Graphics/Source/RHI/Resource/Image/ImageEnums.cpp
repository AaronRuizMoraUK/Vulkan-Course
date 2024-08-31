#include <RHI/Resource/Image/Image.h>

namespace Vulkan
{
    const char* ImageTypeStr(ImageType textureType)
    {
        switch (textureType)
        {
        case ImageType::Image1D:
            return "1D";
        case ImageType::Image2D:
            return "2D";
        case ImageType::Image3D:
            return "3D";
        default:
            return "Unknown";
        }
    }
} // namespace Vulkan
