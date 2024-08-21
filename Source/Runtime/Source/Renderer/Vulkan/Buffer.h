#pragma once

#include <Renderer/Vulkan/BufferDesc.h>

typedef struct VkBuffer_T* VkBuffer;
typedef struct VkDeviceMemory_T* VkDeviceMemory;

namespace Vulkan
{
    class Device;

    // Manages a Vulkan Buffer
    class Buffer
    {
    public:
        Buffer(Device* device, const BufferDesc& desc);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        bool Initialize();
        void Terminate();

        const BufferDesc& GetBufferDesc() const { return m_desc; }

        VkBuffer GetVkBuffer();

    private:
        Device* m_device = nullptr;
        BufferDesc m_desc;

    private:
        bool CreateVkBuffer();

        VkBuffer m_vkBuffer = nullptr;
        VkDeviceMemory m_vkBufferMemory = nullptr;
    };
} // namespace Vulkan
