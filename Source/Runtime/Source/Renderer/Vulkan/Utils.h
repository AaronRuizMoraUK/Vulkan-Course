#pragma once

#include <Renderer/Vulkan/BufferEnums.h>
#include <Renderer/Vulkan/CommandBufferEnums.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags);

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags);

} // namespace Vulkan
