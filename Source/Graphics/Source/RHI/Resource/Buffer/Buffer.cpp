#include <RHI/Resource/Buffer/Buffer.h>

#include <RHI/Device/Device.h>
#include <RHI/CommandBuffer/CommandBuffer.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Vulkan
{
    namespace Utils
    {
        bool CreateVkBuffer(Device* device,
            size_t bufferSize,
            VkBufferUsageFlags vkBufferUsageFlags, 
            VkMemoryPropertyFlags vkMemoryPropertyFlags,
            VkBuffer* vkBufferOut,
            VkDeviceMemory* vkBufferMemoryOut)
        {
            // Create Buffer object
            {
                // If queue families use different queues, then buffer must be shared between families.
                const std::vector<uint32_t>& uniqueFamilyIndices = device->GetQueueFamilyInfo().m_uniqueQueueFamilyIndices;

                VkBufferCreateInfo vkBufferCreateInfo = {};
                vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                vkBufferCreateInfo.pNext = nullptr;
                vkBufferCreateInfo.flags = 0;
                vkBufferCreateInfo.size = bufferSize;
                vkBufferCreateInfo.usage = vkBufferUsageFlags;
                if (uniqueFamilyIndices.size() > 1)
                {
                    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
                    vkBufferCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(uniqueFamilyIndices.size());
                    vkBufferCreateInfo.pQueueFamilyIndices = uniqueFamilyIndices.data();
                }
                else
                {
                    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    vkBufferCreateInfo.queueFamilyIndexCount = 0;
                    vkBufferCreateInfo.pQueueFamilyIndices = nullptr;
                }

                if (vkCreateBuffer(device->GetVkDevice(), &vkBufferCreateInfo, nullptr, vkBufferOut) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Buffer", "Failed to create Vulkan Buffer.");
                    return false;
                }
            }

            // Allocate memory for buffer and link them together
            {
                // Get Buffer's Memory Requirements
                VkMemoryRequirements vkMemoryRequirements = {};
                vkGetBufferMemoryRequirements(device->GetVkDevice(), *vkBufferOut, &vkMemoryRequirements);

                // Allocate memory for the buffer
                VkMemoryAllocateInfo vkMemoryAllocateInfo = {};
                vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                vkMemoryAllocateInfo.pNext = nullptr;
                vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
                vkMemoryAllocateInfo.memoryTypeIndex = FindCompatibleMemoryTypeIndex(
                    device->GetVkPhysicalDevice(), vkMemoryRequirements.memoryTypeBits, vkMemoryPropertyFlags);

                if (vkAllocateMemory(device->GetVkDevice(), &vkMemoryAllocateInfo, nullptr, vkBufferMemoryOut) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Buffer", "Failed to allocate memory for Vulkan Buffer.");
                    return false;
                }

                // Link the buffer to the memory
                if (vkBindBufferMemory(device->GetVkDevice(), *vkBufferOut, *vkBufferMemoryOut, 0) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Buffer", "Failed to bind Vulkan buffer to memory.");
                    return false;
                }
            }

            return true;
        }

        void DestroyVkBuffer(Device* device, VkBuffer& vkBuffer, VkDeviceMemory& vkBufferMemory)
        {
            vkDestroyBuffer(device->GetVkDevice(), vkBuffer, nullptr);
            vkBuffer = nullptr;

            vkFreeMemory(device->GetVkDevice(), vkBufferMemory, nullptr);
            vkBufferMemory = nullptr;
        }

        bool CopyBuffer(Device* device, Buffer* dstBuffer, Buffer* srcBuffer)
        {
            // Command buffer for transfer commands.
            // By Vulkan standards, graphical queues also support transfer commands.
            CommandBuffer transferCmdBuffer(device, 
                device->GetVkCommandPool(QueueFamilyType_Graphics, ResourceTransferCommandPoolIndex));
            if (transferCmdBuffer.Initialize())
            {
                // Record transfer commands to the command buffer.
                // Since it's a transfer operation, there is no need to bind render pass or pipelines.
                // NOTE: To be more accurate, there is an implicit pipeline of only 1 stage which is to do the transfer.
                if (transferCmdBuffer.Begin(CommandBufferUsage_OneTimeSubmit))
                {
                    transferCmdBuffer.CopyBuffer(dstBuffer, srcBuffer);

                    transferCmdBuffer.End();
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            // Execute the transfer command buffer
            {
                VkCommandBuffer vkTransfercCommandBuffer = transferCmdBuffer.GetVkCommandBuffer();

                VkSubmitInfo vkSubmitInfo = {};
                vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                vkSubmitInfo.pNext = nullptr;
                vkSubmitInfo.pWaitDstStageMask = nullptr;
                vkSubmitInfo.waitSemaphoreCount = 0;
                vkSubmitInfo.pWaitSemaphores = nullptr;
                vkSubmitInfo.commandBufferCount = 1;
                vkSubmitInfo.pCommandBuffers = &vkTransfercCommandBuffer;
                vkSubmitInfo.signalSemaphoreCount = 0;
                vkSubmitInfo.pSignalSemaphores = nullptr;

                // Submit command buffer to queue for execution by the GPU
                if (vkQueueSubmit(device->GetVkQueue(Vulkan::QueueFamilyType_Graphics),
                    1, &vkSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Buffer", "Failed to submit transfer work to the queue.");
                    return false;
                }

                // Wait until the queue has finished executing all its commands.
                // 
                // NOTE: This is a sequential way to transfer buffers. More advance techniques
                //       would not block and allow to continue adding transfer commands for other
                //       buffers and synchronize at some point in the future.
                vkQueueWaitIdle(device->GetVkQueue(Vulkan::QueueFamilyType_Graphics));
            }

            return true;
        }

        bool CopyDataToVkBufferMemory(Device* device, VkDeviceMemory vkBufferMemory, const void* bufferData, size_t bufferSize)
        {
            void* dstData = nullptr;

            if (vkMapMemory(device->GetVkDevice(), vkBufferMemory, 0, bufferSize, 0, &dstData) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Buffer", "Failed to map Vulkan buffer memory.");
                return false;
            }

            memcpy(dstData, bufferData, bufferSize);

            vkUnmapMemory(device->GetVkDevice(), vkBufferMemory);

            // NOTE: No need to flush and invalidate since the device memory has the flag VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            return true;
        }
    } // namespace Utils

    Buffer::Buffer(Device* device, const BufferDesc& desc)
        : m_device(device)
        , m_desc(desc)
    {
    }

    Buffer::~Buffer()
    {
        Terminate();
    }

    bool Buffer::Initialize()
    {
        if (m_vkBuffer)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan Buffer", "Initializing Vulkan Buffer...");

        if (!CreateVkBuffer())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Buffer::Terminate()
    {
        DX_LOG(Info, "Vulkan Buffer", "Terminating Vulkan Buffer...");

        Utils::DestroyVkBuffer(m_device, m_vkBuffer, m_vkBufferMemory);
    }

    VkBuffer Buffer::GetVkBuffer()
    {
        return m_vkBuffer;
    }

    bool Buffer::UpdateBufferData(const void* data, size_t dataSize)
    {
        if (m_desc.m_memoryProperty != ResourceMemoryProperty::HostVisible)
        {
            DX_LOG(Error, "Vulkan Buffer", "Only Host Visible buffers can update its data.");
            return false;
        }
        else if (dataSize > m_desc.m_elementSizeInBytes * m_desc.m_elementCount)
        {
            DX_LOG(Error, "Vulkan Buffer", "Trying to copy more data (%d bytes) than buffer's size (%d).",
                dataSize, m_desc.m_elementSizeInBytes * m_desc.m_elementCount);
            return false;
        }

        if (!Utils::CopyDataToVkBufferMemory(m_device, m_vkBufferMemory, data, dataSize))
        {
            DX_LOG(Error, "Vulkan Buffer", "Failed to copy data to Vulkan buffer memory.");
            return false;
        }

        return true;
    }

    bool Buffer::CreateVkBuffer()
    {
        if (m_desc.m_usageFlags == 0)
        {
            DX_LOG(Error, "Vulkan Buffer", "Buffer description with no usage flag set.");
            return false;
        }

        const size_t bufferSize = m_desc.m_elementSizeInBytes * m_desc.m_elementCount;

        switch (m_desc.m_memoryProperty)
        {
        case ResourceMemoryProperty::HostVisible:
        {
            const VkBufferUsageFlags vkBufferUsageFlags = ToVkBufferUsageFlags(m_desc.m_usageFlags);

            // Memory properties we want:
            // - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: visible to the CPU (not optimal for GPU performance)
            // - VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: it causes the data (after being mapped) to be placed straight into
            // the buffer. This removes the need to flush (vkFlushMappedMemoryRanges) and invalidate (vkInvalidateMappedMemoryRanges)
            // the memory after doing map-memcpy-unmap into the buffer.
            const VkMemoryPropertyFlags vkMemoryProperties =
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            if (!Utils::CreateVkBuffer(m_device, bufferSize,
                vkBufferUsageFlags, vkMemoryProperties, &m_vkBuffer, &m_vkBufferMemory))
            {
                return false;
            }

            // Copy data to buffer
            if (m_desc.m_initialData)
            {
                if (!Utils::CopyDataToVkBufferMemory(m_device, m_vkBufferMemory, m_desc.m_initialData, bufferSize))
                {
                    DX_LOG(Error, "Vulkan Buffer", "Failed to map Vulkan buffer memory.");
                    return false;
                }
            }
            break;
        }

        case ResourceMemoryProperty::DeviceLocal:
        {
            // If there is initial data to copy, use a staging buffer to transfer the data to the GPU buffer
            if (m_desc.m_initialData)
            {
                // Create staging source buffer to put data before transferring to GPU
                std::unique_ptr<Buffer> stageBuffer;
                {
                    BufferDesc stageBufferDesc = {};
                    stageBufferDesc.m_elementSizeInBytes = m_desc.m_elementSizeInBytes;
                    stageBufferDesc.m_elementCount = m_desc.m_elementCount;
                    stageBufferDesc.m_usageFlags = BufferUsage_TransferSrc; // Source of the transfer
                    stageBufferDesc.m_memoryProperty = ResourceMemoryProperty::HostVisible;
                    stageBufferDesc.m_initialData = m_desc.m_initialData;

                    stageBuffer = std::make_unique<Buffer>(m_device, stageBufferDesc);
                    if (!stageBuffer->Initialize())
                    {
                        DX_LOG(Error, "Vulkan Buffer", "Failed to create Vulkan staging buffer.");
                        return false;
                    }
                }

                // Create destination buffer in GPU
                {
                    // This buffer will be the target of a transfer require, so adding the
                    // transfer destination flag on top of the actual usage.
                    m_desc.m_usageFlags |= BufferUsage_TransferDst;

                    const VkBufferUsageFlags vkBufferUsageFlags = ToVkBufferUsageFlags(m_desc.m_usageFlags);

                    // Memory properties we want:
                    // - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: visible to the GPU only (optimal for GPU performance)
                    const VkMemoryPropertyFlags vkMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

                    if (!Utils::CreateVkBuffer(m_device, bufferSize,
                        vkBufferUsageFlags, vkMemoryProperties, &m_vkBuffer, &m_vkBufferMemory))
                    {
                        return false;
                    }
                }

                // Execute commands to copy staging buffer to destination buffer on GPU
                if (!Utils::CopyBuffer(m_device, this, stageBuffer.get()))
                {
                    return false;
                }
            }
            // It there is no initial data to copy, just create buffer in GPU
            else
            {
                const VkBufferUsageFlags vkBufferUsageFlags = ToVkBufferUsageFlags(m_desc.m_usageFlags);

                const VkMemoryPropertyFlags vkMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

                if (!Utils::CreateVkBuffer(m_device, bufferSize,
                    vkBufferUsageFlags, vkMemoryProperties, &m_vkBuffer, &m_vkBufferMemory))
                {
                    return false;
                }
            }
            break;
        }

        default:
            DX_LOG(Fatal, "Vulkan Buffer", "Unexpected resource memory property %d.", m_desc.m_memoryProperty);
            return false;
        }

        return true;
    }
} // namespace Vulkan
