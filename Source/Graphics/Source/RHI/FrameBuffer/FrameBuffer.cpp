#include <RHI/FrameBuffer/FrameBuffer.h>

#include <RHI/Device/Device.h>
#include <RHI/RenderPass/RenderPass.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Resource/ImageView/ImageView.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <algorithm>

namespace Vulkan
{
    FrameBuffer::FrameBuffer(Device* device, const FrameBufferDesc& desc)
        : m_device(device)
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

        m_depthStencilImageView.reset();
        // Depth image is destroyed when m_desc.m_depthStencilAttachment.m_image
        // gets out of scope and the shared_ptr counter gets to zero

        m_colorImageViews.clear();
        // Color images are destroyed when m_desc.m_colorAttachments[].m_image
        // gets out of scope and the shared_ptr counter gets to zero
    }

    const FrameBufferDesc& FrameBuffer::GetFrameBufferDesc() const
    {
        return m_desc;
    }

    const Math::Vector2Int& FrameBuffer::GetDimensions() const
    {
        return m_dimensions;
    }

    VkFramebuffer FrameBuffer::GetVkFrameBuffer()
    {
        return m_vkFrameBuffer;
    }

    bool FrameBuffer::CreateColorAttachments()
    {
        m_colorImageViews.reserve(m_desc.m_colorAttachments.size());

        for (auto& colorAttachment : m_desc.m_colorAttachments)
        {
            ImageViewDesc imageViewDesc = {};
            imageViewDesc.m_image = colorAttachment.m_image;
            imageViewDesc.m_viewFormat = colorAttachment.m_viewFormat;
            imageViewDesc.m_aspectFlags = ImageViewAspect_Color;
            imageViewDesc.m_firstMip = 0;
            imageViewDesc.m_mipCount = 0;

            auto colorImageView = std::make_unique<ImageView>(m_device, imageViewDesc);
            if (!colorImageView->Initialize())
            {
                DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Image View for Color Attachment.");
                return false;
            }

            m_colorImageViews.push_back(std::move(colorImageView));
        }

        return true;
    }

    bool FrameBuffer::CreateDepthStencilAttachment()
    {
        if (m_desc.m_depthStencilAttachment.m_image)
        {
            ImageViewDesc imageViewDesc = {};
            imageViewDesc.m_image = m_desc.m_depthStencilAttachment.m_image;
            imageViewDesc.m_viewFormat = m_desc.m_depthStencilAttachment.m_viewFormat;
            imageViewDesc.m_aspectFlags = ImageViewAspect_Depth | ImageViewAspect_Stencil;
            imageViewDesc.m_firstMip = 0;
            imageViewDesc.m_mipCount = 0;

            m_depthStencilImageView = std::make_unique<ImageView>(m_device, imageViewDesc);
            if (!m_depthStencilImageView->Initialize())
            {
                DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Image View for DepthStencil Attachment.");
                return false;
            }
        }

        return true;
    }

    bool FrameBuffer::CreateVkFrameBuffer()
    {
        // Using first the dimensions of first attachment for the frame buffer dimensions
        {
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
        }

        // List of attachment (must match 1:1 with Render Pass attachments)
        std::vector<VkImageView> attachments;
        attachments.reserve(m_colorImageViews.size() + 1); // + 1 in case there is a depth stencil view
        for (const auto& colorImageView : m_colorImageViews)
        {
            attachments.push_back(colorImageView->GetVkImageView());
        }
        if (m_depthStencilImageView)
        {
            attachments.push_back(m_depthStencilImageView->GetVkImageView());
        }

        VkFramebufferCreateInfo vkFramebufferCreateInfo = {};
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.pNext = nullptr;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.renderPass = m_desc.m_renderPass->GetVkRenderPass(); // Render pass this FrameBuffer can be used with
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
            (!m_colorImageViews.empty()) ? "YES" : "NO", m_colorImageViews.size(),
            (m_depthStencilImageView) ? "YES" : "NO");

        return true;
    }
} // namespace Vulkan
