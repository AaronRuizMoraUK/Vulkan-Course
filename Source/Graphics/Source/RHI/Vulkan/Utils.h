#pragma once

#include <RHI/Resource/ResourceEnums.h>
#include <RHI/Resource/Image/ImageEnums.h>
#include <RHI/Resource/Buffer/BufferEnums.h>
#include <RHI/CommandBuffer/CommandBufferEnums.h>
#include <RHI/Shader/ShaderEnums.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Vulkan
{
    uint32_t FindCompatibleMemoryTypeIndex(
        VkPhysicalDevice vkPhysicalDevice, uint32_t allowedMemoryTypes, VkMemoryPropertyFlags properties);

    ResourceFormat ChooseSupportedFormat(VkPhysicalDevice vkPhysicalDevice,
        const std::vector<ResourceFormat>& formats, ImageTiling imageTiling, VkFormatFeatureFlags vkFormatFeatureFlags);

    VkFormat ToVkFormat(ResourceFormat format);

    ResourceFormat ToResourceFormat(VkFormat vkFormat);

    VkImageType ToVkImageType(ImageType imageType);

    VkImageTiling ToVkImageTiling(ImageTiling imageTiling);

    VkImageUsageFlags ToVkImageUsageFlags(ImageUsageFlags flags);

    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags);

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags);

    VkShaderStageFlags ToVkShaderStageFlags(ShaderType shaderType);

} // namespace Vulkan
