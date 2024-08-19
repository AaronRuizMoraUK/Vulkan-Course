#pragma once

#include <Renderer/Vulkan/Image.h>

typedef struct VkFramebuffer_T* VkFramebuffer;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkImageView_T* VkImageView;

namespace Vulkan
{
    class Device;

    // Manages the Vulkan Frame Buffer
    class FrameBuffer
    {
    public:
        FrameBuffer(Device* device, VkRenderPass vkRenderPass, const Image& colorImage, const Image& depthImage = {});
        ~FrameBuffer();

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        bool Initialize(bool createDepthAttachment = false);
        void Terminate();

    private:
        Device* m_device = nullptr;
        VkRenderPass m_vkRenderPass = nullptr;
        Image m_colorImage;

    private:
        bool CreateColorAttachment();
        bool CreateDepthAttachment();
        bool CreateVkFrameBuffer();

        VkImageView m_vkColorImageView = nullptr;

        Image m_depthImage;
        VkImageView m_vkDepthImageView = nullptr;

        VkFramebuffer m_vkFrameBuffer = nullptr;
    };
} // namespace Vulkan
