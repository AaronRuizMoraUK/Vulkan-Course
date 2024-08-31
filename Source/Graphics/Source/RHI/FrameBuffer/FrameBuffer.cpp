#include <RHI/FrameBuffer/FrameBuffer.h>

#include <RHI/Device/Device.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <algorithm>

namespace Vulkan
{
    namespace Utils
    {
        bool CreateVkImageView(
            Device* device, 
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

            if (vkCreateImageView(device->GetVkDevice(), &vkImageViewCreateInfo, nullptr, vkImageViewOut) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image View.");
                return false;
            }

            return true;
        }

        void DestroyVkImageView(Device* device, VkImageView& vkImageView)
        {
            vkDestroyImageView(device->GetVkDevice(), vkImageView, nullptr);
            vkImageView = nullptr;
        }
    } // namespace Utils

    FrameBuffer::FrameBuffer(Device* device, VkRenderPass vkRenderPass, const FrameBufferDesc& desc)
        : m_device(device)
        , m_vkRenderPass(vkRenderPass)
        , m_desc(desc)
    {
    }

    FrameBuffer::~FrameBuffer()
    {
        Terminate();
    }

    bool FrameBuffer::Initialize()
    {
        if (m_vkFrameBuffer)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan FrameBuffer", "Initializing Vulkan FrameBuffer...");

        if (!CreateColorAttachments())
        {
            Terminate();
            return false;
        }

        if (!CreateDepthStencilAttachment())
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

        Utils::DestroyVkImageView(m_device, m_vkDepthStencilImageView);
        // Depth image is destroyed when m_desc.m_depthStencilAttachment.m_image
        // gets out of scope and the shared_ptr counter gets to zero

        std::ranges::for_each(m_vkColorImageViews, [this](VkImageView vkImageView)
            {
                Utils::DestroyVkImageView(m_device, vkImageView);
            });
        m_vkColorImageViews.clear();
        // Color images are destroyed when m_desc.m_colorAttachments[].m_image
        // gets out of scope and the shared_ptr counter gets to zero
    }

    const Math::Vector2Int& FrameBuffer::GetDimensions() const
    {
        return m_dimensions;
    }

    VkRenderPass FrameBuffer::GetVkRenderPass()
    {
        return m_vkRenderPass;
    }

    VkFramebuffer FrameBuffer::GetVkFrameBuffer()
    {
        return m_vkFrameBuffer;
    }

    bool FrameBuffer::CreateColorAttachments()
    {
        m_vkColorImageViews.reserve(m_desc.m_colorAttachments.size());

        for (auto& colorAttachment : m_desc.m_colorAttachments)
        {
            VkImageView vkColorImageView = nullptr;
            if (!Utils::CreateVkImageView(m_device, 
                colorAttachment.m_image->GetVkImage(),
                ToVkFormat(colorAttachment.m_viewFormat),
                VK_IMAGE_ASPECT_COLOR_BIT, 
                &vkColorImageView))
            {
                DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image View for Color Attachment.");
                return false;
            }

            m_vkColorImageViews.push_back(vkColorImageView);
        }

        return true;
    }

    bool FrameBuffer::CreateDepthStencilAttachment()
    {
        if (!m_desc.m_depthStencilAttachment.m_image &&
            m_desc.m_createDepthStencilAttachment)
        {
            if (!m_desc.m_colorAttachments.empty())
            {
                const ImageDesc& imageDesc = m_desc.m_colorAttachments.front().m_image->GetImageDesc();

                ImageDesc depthStencilImageDesc = {};
                depthStencilImageDesc.m_imageType = ImageType::Image2D;
                depthStencilImageDesc.m_dimensions = imageDesc.m_dimensions;
                depthStencilImageDesc.m_mipCount = imageDesc.m_mipCount;
                depthStencilImageDesc.m_format = ResourceFormat::D24_UNORM_S8_UINT;
                //depthStencilImageDesc.m_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

                // NOTE: Not created through owner device API to avoid having a
                // reference in the device as this is a sub-object of FrameBuffer.
                m_desc.m_depthStencilAttachment.m_image = std::make_shared<Image>(m_device, depthStencilImageDesc);
                if (!m_desc.m_depthStencilAttachment.m_image->Initialize())
                {
                    DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image for Depth Attachment.");
                    return false;
                }
                m_desc.m_depthStencilAttachment.m_viewFormat = depthStencilImageDesc.m_format;
            }
        }

        if (m_desc.m_depthStencilAttachment.m_image)
        {
            if (!Utils::CreateVkImageView(m_device,
                m_desc.m_depthStencilAttachment.m_image->GetVkImage(),
                ToVkFormat(m_desc.m_depthStencilAttachment.m_viewFormat),
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                &m_vkDepthStencilImageView))
            {
                DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan Image View for Depth Attachment.");
                return false;
            }
        }

        return true;
    }

    bool FrameBuffer::CreateVkFrameBuffer()
    {
        // Using first the dimensions of first attachment for the frame buffer dimensions
        if (!m_desc.m_colorAttachments.empty())
        {
            const auto& colorDimensions = m_desc.m_colorAttachments.front().m_image->GetImageDesc().m_dimensions;
            m_dimensions = Math::Vector2Int(colorDimensions.x, colorDimensions.y);
        }
        else if (m_desc.m_depthStencilAttachment.m_image)
        {
            const auto& depthStencilDimensions = m_desc.m_depthStencilAttachment.m_image->GetImageDesc().m_dimensions;
            m_dimensions = Math::Vector2Int(depthStencilDimensions.x, depthStencilDimensions.y);
        }
        else
        {
            DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan FrameBuffer as there are no attachments.");
            return false;
        }

        // List of attachment (1:1 with Render Pass)
        std::vector<VkImageView> attachments = m_vkColorImageViews;
        if (m_vkDepthStencilImageView)
        {
            attachments.push_back(m_vkDepthStencilImageView);
        }

        VkFramebufferCreateInfo vkFramebufferCreateInfo = {};
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.pNext = nullptr;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.renderPass = m_vkRenderPass; // Render pass this FrameBuffer can be used with
        vkFramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        vkFramebufferCreateInfo.pAttachments = attachments.data();
        vkFramebufferCreateInfo.width = m_dimensions.x;
        vkFramebufferCreateInfo.height = m_dimensions.y;
        vkFramebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(
            m_device->GetVkDevice(), &vkFramebufferCreateInfo, nullptr, &m_vkFrameBuffer) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan FrameBuffer.");
            return false;
        }

        DX_LOG(Info, "Vulkan FrameBuffer", "Frame buffer created. Color: %s (%d) DepthStencil: %s",
            (!m_vkColorImageViews.empty()) ? "YES" : "NO", m_vkColorImageViews.size(),
            (m_vkDepthStencilImageView) ? "YES" : "NO");

        return true;
    }
} // namespace Vulkan
