#pragma once

#include <Renderer/Vulkan/BufferEnums.h>
#include <Renderer/Vulkan/CommandBufferEnums.h>
#include <Renderer/Vulkan/ShaderEnums.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags);

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags);

    VkShaderStageFlags ToVkShaderStageFlags(ShaderType shaderType);

} // namespace Vulkan
