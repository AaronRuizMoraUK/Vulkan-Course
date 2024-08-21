#include <Renderer/Vulkan/Utils.h>

namespace Vulkan
{

    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags usageFlags)
    {
        VkBufferUsageFlags vkBufferUsageFlags = 0;

        vkBufferUsageFlags |= (usageFlags & BufferUsage_VertexBuffer) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
        vkBufferUsageFlags |= (usageFlags & BufferUsage_IndexBuffer) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;

        return vkBufferUsageFlags;
    }

} // namespace Vulkan
