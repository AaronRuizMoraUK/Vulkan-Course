#pragma once

#include <RHI/Resource/Image/ImageDesc.h>

typedef struct VkImage_T* VkImage;
typedef struct VkDeviceMemory_T* VkDeviceMemory;

namespace Vulkan
{
    class Device;

    // Manages a Vulkan Image
    class Image
    {
    public:
        Image(Device* device, const ImageDesc& desc);
        ~Image();

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        bool Initialize();
        void Terminate();

        const ImageDesc& GetImageDesc() const { return m_desc; }

        VkImage GetVkImage();

    private:
        Device* m_device = nullptr;
        ImageDesc m_desc;

    private:
        uint32_t CalculateImageMemorySize() const;
        bool CreateVkImage();

        VkImage m_vkImage = nullptr;
        VkDeviceMemory m_vkImageMemory = nullptr;
    };
} // namespace Vulkan
