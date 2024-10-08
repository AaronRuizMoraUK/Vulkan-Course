#pragma once

#include <RHI/FrameBuffer/FrameBufferDesc.h>

#include <Math/Vector2.h>

#include <vector>

typedef struct VkFramebuffer_T* VkFramebuffer;
typedef struct VkImageView_T* VkImageView;

namespace Vulkan
{
    class Device;
    class ImageView;

    // Manages the Vulkan Frame Buffer
    class FrameBuffer
    {
    public:
        FrameBuffer(Device* device, const FrameBufferDesc& desc);
        ~FrameBuffer();

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        bool Initialize();
        void Terminate();

        const FrameBufferDesc& GetFrameBufferDesc() const;
        const Math::Vector2Int& GetDimensions() const;

        ImageView* GetImageView(uint32_t attachmentIndex);
        VkFramebuffer GetVkFrameBuffer();

    private:
        Device* m_device = nullptr;
        FrameBufferDesc m_desc;

    private:
        bool CreateVkFrameBuffer();

        Math::Vector2Int m_dimensions;

        std::vector<std::unique_ptr<ImageView>> m_imageViews;

        VkFramebuffer m_vkFrameBuffer = nullptr;
    };
} // namespace Vulkan
