#pragma once

#include <RHI/Resource/Buffer/BufferEnums.h>
#include <RHI/CommandBuffer/CommandBufferEnums.h>
#include <RHI/Shader/ShaderEnums.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags);

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags);

    VkShaderStageFlags ToVkShaderStageFlags(ShaderType shaderType);

} // namespace Vulkan
