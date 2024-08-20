#include <Renderer/Vulkan/CommandBuffer.h>

#include <Renderer/Vulkan/Device.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    CommandBuffer::CommandBuffer(Device* device, VkCommandPool vkCommandPool)
        : m_device(device)
        , m_vkCommandPool(vkCommandPool)
    {
    }

    CommandBuffer::~CommandBuffer()
    {
        Terminate();
    }

    bool CommandBuffer::Initialize([[maybe_unused]] bool createDepthAttachment)
    {
        if (m_vkCommandBuffer)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan CommandBuffer", "Initializing Vulkan CommandBuffer...");

        if (!AllocateVkCommandBuffer())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void CommandBuffer::Terminate()
    {
        DX_LOG(Info, "Vulkan CommandBuffer", "Terminating Vulkan CommandBuffer...");

        vkFreeCommandBuffers(m_device->GetVkDevice(), m_vkCommandPool, 1, &m_vkCommandBuffer);
        m_vkCommandBuffer = nullptr;
    }

    bool CommandBuffer::AllocateVkCommandBuffer()
    {
        VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo = {};
        vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        vkCommandBufferAllocateInfo.pNext = nullptr;
        vkCommandBufferAllocateInfo.commandPool = m_vkCommandPool;
        vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Primary cmd buffers submit directly to queue, secondary to other cmd buffers.
        vkCommandBufferAllocateInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(
            m_device->GetVkDevice(), &vkCommandBufferAllocateInfo, &m_vkCommandBuffer) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan CommandBuffer", "Failed to create Vulkan CommandBuffer.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
