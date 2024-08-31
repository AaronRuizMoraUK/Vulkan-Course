#pragma once

#include <RHI/Resource/ResourceEnums.h>
#include <RHI/Resource/Buffer/BufferEnums.h>
#include <RHI/CommandBuffer/CommandBufferEnums.h>
#include <RHI/Shader/ShaderEnums.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    VkFormat ToVkFormat(ResourceFormat format);

    ResourceFormat ToResourceFormat(VkFormat vkFormat);

    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags);

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags);

    VkShaderStageFlags ToVkShaderStageFlags(ShaderType shaderType);

} // namespace Vulkan
