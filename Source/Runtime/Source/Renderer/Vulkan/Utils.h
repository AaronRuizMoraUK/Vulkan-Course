#pragma once

#include <Renderer/Vulkan/BufferEnums.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags usageFlags);

} // namespace Vulkan
