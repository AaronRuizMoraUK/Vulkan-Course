#include <Renderer/Vulkan/Buffer.h>

#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/CommandBuffer.h>
#include <Renderer/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Vulkan
{
    namespace Utils
    {
        void DestroyVkBuffer(Device* device, VkBuffer& vkBuffer, VkDeviceMemory& vkBufferMemory);

        // Helper to destroy stage buffer when getting out of scope.
        struct ScopedBuffer
        {
            VkBuffer m_vkBuffer = nullptr;
            VkDeviceMemory m_vkBufferMemory = nullptr;

            ScopedBuffer(Device* device)
                : m_device(device)
            {
            }
            ~ScopedBuffer()
            {
                DestroyVkBuffer(m_device, m_vkBuffer, m_vkBufferMemory);
            }
        private:
            Device* m_device = nullptr;
        };

        // Finds the index of the memory type which is in the allowed list and has all the properties passed by argument.
        uint32_t FindCompatibleMemoryTypeIndex(
            VkPhysicalDevice vkPhysicalDevice, uint32_t allowedMemoryTypes, VkMemoryPropertyFlags properties)
        {
            // Get properties of the physical device memory
            VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties = {};
            vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkPhysicalDeviceMemoryProperties);

            // For each memory type
            for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
            {
                // allowedMemoryTypes is a bit field where each bit is an allowed type,
                // since it's an uint32_t (32 bits) that's 32 different types possible.
                // First one would be the first bit, second would be second bit and so on. 
                const bool allowedMemoryType = (allowedMemoryTypes & (1 << i));

                const bool supportsProperties = 
                    (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties;

                if (allowedMemoryType && supportsProperties)
                {
                    return i;
                }
            }

            DX_LOG(Warning, "Vulkan Buffer", "Compatible memory for buffer not found!");
            return std::numeric_limits<uint32_t>::max();
        }

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

            // Allocate memory for buffer and bind them together
            {
                // Get Buffer's Memory Requirements
                VkMemoryRequirements vkMemoryRequirements = {};
                vkGetBufferMemoryRequirements(device->GetVkDevice(), *vkBufferOut, &vkMemoryRequirements);

                // Allocate memory for the buffer
                VkMemoryAllocateInfo vkMemoryAllocateInfo = {};
                vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                vkMemoryAllocateInfo.pNext = nullptr;
                vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
                vkMemoryAllocateInfo.memoryTypeIndex = Utils::FindCompatibleMemoryTypeIndex(
                    device->GetVkPhysicalDevice(), vkMemoryRequirements.memoryTypeBits, vkMemoryPropertyFlags);

                if (vkAllocateMemory(device->GetVkDevice(), &vkMemoryAllocateInfo, nullptr, vkBufferMemoryOut) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Buffer", "Failed to allocate memory for Vulkan Buffer.");
                    return false;
                }

                // Bind the buffer to the memory
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

        bool CopyVkBuffer(Device* device, VkBuffer vkDstBuffer, VkBuffer vkSrcBuffer, size_t bufferSize)
        {
            // Command buffer for transfer commands.
            // By Vulkan standards, graphical queues also support transfer commands.
            CommandBuffer transferCmdBuffer(device, device->GetVkCommandPool(QueueFamilyType_Graphics));
            if (transferCmdBuffer.Initialize())
            {
                // Record transfer commands to the command buffer.
                // Since it's a transfer operation, there is no need to bind render pass or pipelines.
                if (transferCmdBuffer.Begin(CommandBufferUsage_OneTimeSubmit))
                {
                    transferCmdBuffer.CopyBuffer(vkDstBuffer, vkSrcBuffer, bufferSize);

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
                // Pass the render fence of the current frame, so when it's finished drawing it will signal it.
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
        if (m_desc.m_memoryProperty != BufferMemoryProperty::HostVisible)
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
            DX_LOG(Error, "Vulkan Buffer", "Failed to map Vulkan buffer memory.");
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
        case BufferMemoryProperty::HostVisible:
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

        case BufferMemoryProperty::DeviceLocal:
        {
            // If there is initial data to copy, use a staging buffer to transfer the data to the GPU buffer
            if (m_desc.m_initialData)
            {
                // Create staging source buffer to put data before transferring to GPU
                Utils::ScopedBuffer stageBuffer(m_device);
                {
                    const VkBufferUsageFlags vkBufferUsageFlags =
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // Source of the transfer

                    const VkMemoryPropertyFlags vkMemoryProperties =
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

                    if (!Utils::CreateVkBuffer(m_device, bufferSize,
                        vkBufferUsageFlags, vkMemoryProperties, &stageBuffer.m_vkBuffer, &stageBuffer.m_vkBufferMemory))
                    {
                        return false;
                    }
                }

                // Copy data to "stage" buffer
                if (!Utils::CopyDataToVkBufferMemory(m_device, stageBuffer.m_vkBufferMemory, m_desc.m_initialData, bufferSize))
                {
                    DX_LOG(Error, "Vulkan Buffer", "Failed to map Vulkan buffer memory.");
                    return false;
                }

                // Create destination buffer in GPU
                {
                    const VkBufferUsageFlags vkBufferUsageFlags =
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | // Destination of the transfer
                        ToVkBufferUsageFlags(m_desc.m_usageFlags);

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
                if (!Utils::CopyVkBuffer(m_device, m_vkBuffer, stageBuffer.m_vkBuffer, bufferSize))
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
            DX_LOG(Fatal, "Vulkan Buffer", "Unexpected buffer memory property %d.", m_desc.m_memoryProperty);
            return false;
        }

        return true;
    }
} // namespace Vulkan
