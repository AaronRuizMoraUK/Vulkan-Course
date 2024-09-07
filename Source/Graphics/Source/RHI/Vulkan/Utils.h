#pragma once

#include <RHI/Resource/ResourceEnums.h>
#include <RHI/Resource/Image/ImageEnums.h>
#include <RHI/Resource/ImageView/ImageViewEnums.h>
#include <RHI/Resource/Buffer/BufferEnums.h>
#include <RHI/Sampler/SamplerEnums.h>
#include <RHI/CommandBuffer/CommandBufferEnums.h>
#include <RHI/Shader/ShaderEnums.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Vulkan
{
    uint32_t FindCompatibleMemoryTypeIndex(
        VkPhysicalDevice vkPhysicalDevice, uint32_t allowedMemoryTypes, VkMemoryPropertyFlags properties);

    VkFormat ToVkFormat(ResourceFormat format);

    ResourceFormat ToResourceFormat(VkFormat vkFormat);

    VkImageType ToVkImageType(ImageType imageType);

    VkImageViewType ToVkImageViewType(ImageType imageType);

    VkImageTiling ToVkImageTiling(ImageTiling imageTiling);

    VkImageUsageFlags ToVkImageUsageFlags(ImageUsageFlags flags);

    VkImageAspectFlags ToVkImageAspectFlags(ImageViewAspectFlags flags);

    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags);

    VkFilter ToVkFilter(FilterSampling filter);

    VkSamplerMipmapMode ToVkSamplerMipmapMode(FilterSampling filter);

    VkSamplerAddressMode ToVkSamplerAddressMode(AddressMode addressMode);

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags);

    VkShaderStageFlags ToVkShaderStageFlags(ShaderTypeFlags flags);

} // namespace Vulkan
