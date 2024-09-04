#pragma once

#include <RHI/Resource/ImageView/ImageViewDesc.h>

typedef struct VkImageView_T* VkImageView;

namespace Vulkan
{
    class Device;

    class ImageView
    {
    public:
        ImageView(Device* device, const ImageViewDesc& desc);
        ~ImageView();

        ImageView(const ImageView&) = delete;
        ImageView& operator=(const ImageView&) = delete;

        bool Initialize();
        void Terminate();

        VkImageView GetVkImageView();

    private:
        Device* m_device = nullptr;
        ImageViewDesc m_desc;

    private:
        bool CreateVkImageView();

        VkImageView m_vkImageView = nullptr;
    };
} // namespace Vulkan
