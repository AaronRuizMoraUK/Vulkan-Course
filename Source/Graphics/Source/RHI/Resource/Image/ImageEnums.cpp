#include <RHI/Resource/Image/Image.h>

#include <RHI/Device/Device.h>
#include <RHI/Vulkan/Utils.h>

#include <vulkan/vulkan.h>

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

    ResourceFormat ChooseSupportedFormat(
        Device* device,
        const std::vector<ResourceFormat>& formats,
        ImageTiling imageTiling,
        int vkFormatFeatureFlags)
    {
        // Loop through the formats provided and find a compatible one
        for (ResourceFormat format : formats)
        {
            // Get properties of the format on this device
            VkFormatProperties vkFormatProperties = {};
            vkGetPhysicalDeviceFormatProperties(device->GetVkPhysicalDevice(), ToVkFormat(format), &vkFormatProperties);

            // Does the format support all features we requested for optimal tiling?
            if (imageTiling == ImageTiling::Optimal &&
                (vkFormatProperties.optimalTilingFeatures & vkFormatFeatureFlags) == static_cast<VkFormatFeatureFlags>(vkFormatFeatureFlags))
            {
                return format;
            }
            // Does the format support all features we requested for lineal tiling?
            else if (imageTiling == ImageTiling::Linear &&
                (vkFormatProperties.linearTilingFeatures & vkFormatFeatureFlags) == static_cast<VkFormatFeatureFlags>(vkFormatFeatureFlags))
            {
                return format;
            }
        }

        return ResourceFormat::Unknown;
    }
} // namespace Vulkan
