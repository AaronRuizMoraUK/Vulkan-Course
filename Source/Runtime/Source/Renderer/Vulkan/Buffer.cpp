#include <Renderer/Vulkan/Buffer.h>

#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Vulkan
{
    namespace Utils
    {
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
    }

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

        vkFreeMemory(m_device->GetVkDevice(), m_vkBufferMemory, nullptr);
        m_vkBufferMemory = nullptr;

        vkDestroyBuffer(m_device->GetVkDevice(), m_vkBuffer, nullptr);
        m_vkBuffer = nullptr;
    }

    VkBuffer Buffer::GetVkBuffer()
    {
        return m_vkBuffer;
    }

    bool Buffer::CreateVkBuffer()
    {
        if (m_desc.m_usageFlags == 0)
        {
            DX_LOG(Error, "Vulkan Buffer", "Buffer description with no usage flag set.");
            return false;
        }

        // 1) Create Buffer object
        {
            // If queue families use different queues, then buffer must be shared between families.
            const std::vector<uint32_t>& uniqueFamilyIndices = m_device->GetQueueFamilyInfo().m_uniqueQueueFamilyIndices;

            VkBufferCreateInfo vkBufferCreateInfo = {};
            vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            vkBufferCreateInfo.pNext = nullptr;
            vkBufferCreateInfo.flags = 0;
            vkBufferCreateInfo.size = m_desc.m_elementSizeInBytes * m_desc.m_elementCount;
            vkBufferCreateInfo.usage = ToVkBufferUsageFlags(m_desc.m_usageFlags);
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

            if (vkCreateBuffer(m_device->GetVkDevice(), &vkBufferCreateInfo, nullptr, &m_vkBuffer) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Buffer", "Failed to create Vulkan Buffer.");
                return false;
            }
        }

        // 2) Allocate memory for buffer and bind them together
        {
            // Get Buffer's Memory Requirements
            VkMemoryRequirements vkMemoryRequirements = {};
            vkGetBufferMemoryRequirements(m_device->GetVkDevice(), m_vkBuffer, &vkMemoryRequirements);

            // Memory properties we want:
            // - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: visible to the CPU (not optimal for GPU performance)
            // - VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: it causes the data (after being mapped) to be placed straight into
            // the buffer. This removes the need to flush (vkFlushMappedMemoryRanges) and invalidate (vkInvalidateMappedMemoryRanges)
            // the memory after doing map-memcpy-unmap into the buffer.
            const VkMemoryPropertyFlags vkMemoryProperties =
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            // Allocate memory for the buffer
            VkMemoryAllocateInfo vkMemoryAllocateInfo = {};
            vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            vkMemoryAllocateInfo.pNext = nullptr;
            vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
            vkMemoryAllocateInfo.memoryTypeIndex = Utils::FindCompatibleMemoryTypeIndex(
                m_device->GetVkPhysicalDevice(), vkMemoryRequirements.memoryTypeBits, vkMemoryProperties);

            if (vkAllocateMemory(m_device->GetVkDevice(), &vkMemoryAllocateInfo, nullptr, &m_vkBufferMemory) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Buffer", "Failed to allocate memory for Vulkan Buffer.");
                return false;
            }

            // Bind the buffer to the memory
            if (vkBindBufferMemory(m_device->GetVkDevice(), m_vkBuffer, m_vkBufferMemory, 0) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Buffer", "Failed to bind Vulkan buffer to memory.");
                return false;
            }
        }

        // 3) Copy data to buffer
        if (m_desc.m_initialData)
        {
            const uint32_t dataSize = m_desc.m_elementSizeInBytes * m_desc.m_elementCount;
            void* dstData = nullptr;

            if (vkMapMemory(m_device->GetVkDevice(), m_vkBufferMemory, 0, dataSize, 0, &dstData) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Buffer", "Failed to map Vulkan buffer memory.");
                return false;
            }

            memcpy(dstData, m_desc.m_initialData, dataSize);

            vkUnmapMemory(m_device->GetVkDevice(), m_vkBufferMemory);

            // NOTE: No need to flush and invalidate since the memory has the flag VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        }

        return true;
    }
} // namespace Vulkan
