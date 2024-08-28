#include <Renderer/Vulkan/Utils.h>

namespace Vulkan
{

    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags)
    {
        VkBufferUsageFlags vkBufferUsageFlags = 0;

        vkBufferUsageFlags |= (flags & BufferUsage_VertexBuffer) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
        vkBufferUsageFlags |= (flags & BufferUsage_IndexBuffer) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
        vkBufferUsageFlags |= (flags & BufferUsage_UniformBuffer) ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;

        return vkBufferUsageFlags;
    }

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags)
    {
        VkCommandBufferUsageFlags vkCommandBufferUsageFlags = 0;

        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_OneTimeSubmit) ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_RenderPassContinue) ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0;
        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_SimultaneousUse) ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0;

        return vkCommandBufferUsageFlags;
    }

} // namespace Vulkan
