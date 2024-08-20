#include <Renderer/Vulkan/FrameBuffer.h>

#include <Renderer/Vulkan/Device.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Vulkan
{
    namespace Utils
    {
        bool CreateVkImage(
            VkDevice vkDevice, 
            const std::vector<uint32_t>& uniqueFamilyIndices, 
            const Math::Vector2Int& size, 
            VkFormat vkFormat, 
            VkImageUsageFlags vkImageUsageFlags, 
            VkImage* vkImageOut)
        {
            VkImageCreateInfo vkImageCreateInfo = {};
            vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            vkImageCreateInfo.pNext = nullptr;
            vkImageCreateInfo.flags = 0;
            vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            vkImageCreateInfo.format = vkFormat;
            vkImageCreateInfo.extent.width = size.x;
            vkImageCreateInfo.extent.height = size.y;
            vkImageCreateInfo.extent.depth = 0;
            vkImageCreateInfo.mipLevels = 1;
            vkImageCreateInfo.arrayLayers = 1;
            vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            vkImageCreateInfo.tiling; // TODO
            vkImageCreateInfo.usage = vkImageUsageFlags;
            if (uniqueFamilyIndices.size() > 1)
            {
                vkImageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
                vkImageCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(uniqueFamilyIndices.size());
                vkImageCreateInfo.pQueueFamilyIndices = uniqueFamilyIndices.data();
            }
            else
            {
                vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                vkImageCreateInfo.queueFamilyIndexCount = 0;
                vkImageCreateInfo.pQueueFamilyIndices = nullptr;
            }
            vkImageCreateInfo.initialLayout; // TODO

            if (vkCreateImage(vkDevice, &vkImageCreateInfo, nullptr, vkImageOut) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image.");
                return false;
            }

            return true;
        }

        bool CreateVkImageView(
            VkDevice vkDevice, 
            VkImage vkImage, 
            VkFormat vkFormat, 
            VkImageAspectFlags vkAspectFlags, 
            VkImageView* vkImageViewOut)
        {
            VkImageViewCreateInfo vkImageViewCreateInfo = {};
            vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            vkImageViewCreateInfo.pNext = nullptr;
            vkImageViewCreateInfo.flags = 0;
            vkImageViewCreateInfo.image = vkImage;
            vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            vkImageViewCreateInfo.format = vkFormat;
            vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            // With subresource range it specifies the part of the image to view
            vkImageViewCreateInfo.subresourceRange.aspectMask = vkAspectFlags;
            vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            vkImageViewCreateInfo.subresourceRange.levelCount = 1;
            vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            vkImageViewCreateInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(vkDevice, &vkImageViewCreateInfo, nullptr, vkImageViewOut) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image View.");
                return false;
            }

            return true;
        }
    } // namespace Utils

    FrameBuffer::FrameBuffer(Device* device, VkRenderPass vkRenderPass, const Image& colorImage, const Image& depthImage)
        : m_device(device)
        , m_vkRenderPass(vkRenderPass)
        , m_colorImage(colorImage)
        , m_depthImage(depthImage)
    {
    }

    FrameBuffer::~FrameBuffer()
    {
        Terminate();
    }

    bool FrameBuffer::Initialize([[maybe_unused]] bool createDepthAttachment)
    {
        if (m_vkFrameBuffer)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan FrameBuffer", "Initializing Vulkan FrameBuffer...");

        if (!CreateColorAttachment())
        {
            Terminate();
            return false;
        }

        if (createDepthAttachment && !CreateDepthAttachment())
        {
            Terminate();
            return false;
        }

        if (!CreateVkFrameBuffer())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void FrameBuffer::Terminate()
    {
        DX_LOG(Info, "Vulkan FrameBuffer", "Terminating Vulkan FrameBuffer...");

        vkDestroyFramebuffer(m_device->GetVkDevice(), m_vkFrameBuffer, nullptr);
        m_vkFrameBuffer = nullptr;

        vkDestroyImageView(m_device->GetVkDevice(), m_vkDepthImageView, nullptr);
        vkDestroyImage(m_device->GetVkDevice(), m_depthImage.m_vkImage, nullptr);
        m_vkDepthImageView = nullptr;
        m_depthImage = {};

        vkDestroyImageView(m_device->GetVkDevice(), m_vkColorImageView, nullptr);
        m_vkColorImageView = nullptr;
        // Color image was passed to FrameBuffer, so it's not its responsibility to destroy it.
    }

    VkRenderPass FrameBuffer::GetVkRenderPass()
    {
        return m_vkRenderPass;
    }

    VkFramebuffer FrameBuffer::GetVkFrameBuffer()
    {
        return m_vkFrameBuffer;
    }

    const Image& FrameBuffer::GetColorImage() const
    {
        return m_colorImage;
    }

    const Image& FrameBuffer::GetDepthImage() const
    {
        return m_depthImage;
    }

    bool FrameBuffer::CreateColorAttachment()
    {
        if (!Utils::CreateVkImageView(m_device->GetVkDevice(), m_colorImage.m_vkImage,
            static_cast<VkFormat>(m_colorImage.m_vkFormat), VK_IMAGE_ASPECT_COLOR_BIT, &m_vkColorImageView))
        {
            DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image View for Color Image.");
            return false;
        }

        return true;
    }


    bool FrameBuffer::CreateDepthAttachment()
    {
        const VkFormat vkDepthImageFormat = VK_FORMAT_D24_UNORM_S8_UINT;

        m_depthImage.m_size = m_colorImage.m_size;
        m_depthImage.m_vkFormat = vkDepthImageFormat;

        if (!Utils::CreateVkImage(m_device->GetVkDevice(),
            m_device->GetQueueFamilyInfo().m_uniqueQueueFamilyIndices, m_depthImage.m_size,
            vkDepthImageFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_depthImage.m_vkImage))
        {
            DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image View for Depth Attachment.");
            return false;
        }

        if (!Utils::CreateVkImageView(m_device->GetVkDevice(), m_depthImage.m_vkImage,
            vkDepthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, &m_vkDepthImageView))
        {
            DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image View for Depth Image.");
            return false;
        }

        return true;
    }

    bool FrameBuffer::CreateVkFrameBuffer()
    {
        // List of attachment (1:1 with Render Pass)
        std::vector<VkImageView> attachments = { m_vkColorImageView };
        if (m_vkDepthImageView)
        {
            attachments.push_back(m_vkDepthImageView);
        }

        VkFramebufferCreateInfo vkFramebufferCreateInfo = {};
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.pNext = nullptr;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.renderPass = m_vkRenderPass; // Render pass the FrameBuffer will be used with
        vkFramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        vkFramebufferCreateInfo.pAttachments = attachments.data();
        vkFramebufferCreateInfo.width = m_colorImage.m_size.x;
        vkFramebufferCreateInfo.height = m_colorImage.m_size.y;
        vkFramebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(
            m_device->GetVkDevice(), &vkFramebufferCreateInfo, nullptr, &m_vkFrameBuffer) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan FrameBuffer.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
