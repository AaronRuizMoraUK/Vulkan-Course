#include <RHI/FrameBuffer/FrameBuffer.h>

#include <RHI/Device/Device.h>
#include <RHI/RenderPass/RenderPass.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Resource/ImageView/ImageView.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <numeric>

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

        m_imageViews.clear();
        // Images are destroyed when m_desc.m_attachments[].m_image
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

    ImageView* FrameBuffer::GetImageView(uint32_t attachmentIndex)
    {
        if (attachmentIndex < m_imageViews.size())
        {
            return m_imageViews[attachmentIndex].get();
        }
        else
        {
            return nullptr;
        }
    }

    VkFramebuffer FrameBuffer::GetVkFrameBuffer()
    {
        return m_vkFrameBuffer;
    }

    bool FrameBuffer::CreateVkFrameBuffer()
    {
        if (m_desc.m_attachments.empty())
        {
            DX_LOG(Error, "Vulkan FrameBuffer", "No attachments passed to the frame buffer.");
            return false;
        }

        m_imageViews.reserve(m_desc.m_attachments.size());
        for (auto& attachment : m_desc.m_attachments)
        {
            ImageViewDesc imageViewDesc = {};
            imageViewDesc.m_image = attachment.m_image;
            imageViewDesc.m_viewFormat = attachment.m_viewFormat;
            imageViewDesc.m_aspectFlags = attachment.m_viewAspectFlags;
            imageViewDesc.m_firstMip = 0;
            imageViewDesc.m_mipCount = 0;

            auto imageView = std::make_unique<ImageView>(m_device, imageViewDesc);
            if (!imageView->Initialize())
            {
                DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Image View for Attachment.");
                return false;
            }

            m_imageViews.push_back(std::move(imageView));
        }

        // Using the dimensions of first attachment for the frame buffer dimensions{
        {
            const auto& dimensions = m_desc.m_attachments.front().m_image->GetImageDesc().m_dimensions;
            m_dimensions = Math::Vector2Int(dimensions.x, dimensions.y);
        }

        // List of attachment (must match 1:1 with Render Pass attachments)
        std::vector<VkImageView> vkImageViews;
        vkImageViews.reserve(m_imageViews.size());
        for (const auto& imageView : m_imageViews)
        {
            vkImageViews.push_back(imageView->GetVkImageView());
        }

        VkFramebufferCreateInfo vkFramebufferCreateInfo = {};
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.pNext = nullptr;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.renderPass = m_desc.m_renderPass->GetVkRenderPass(); // Render pass this FrameBuffer can be used with
        vkFramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(vkImageViews.size());
        vkFramebufferCreateInfo.pAttachments = vkImageViews.data();
        vkFramebufferCreateInfo.width = m_dimensions.x;
        vkFramebufferCreateInfo.height = m_dimensions.y;
        vkFramebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(
            m_device->GetVkDevice(), &vkFramebufferCreateInfo, nullptr, &m_vkFrameBuffer) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan FrameBuffer", "Failed to create Vulkan FrameBuffer.");
            return false;
        }

        const int colorCount = std::reduce(m_desc.m_attachments.begin(), m_desc.m_attachments.end(), 0,
            [](int accumulator, const ImageAttachment& imageAttachment)
            {
                return accumulator + (imageAttachment.m_viewAspectFlags & ImageViewAspect_Color) ? 1 : 0;
            });
        const int depthStencilCount = std::reduce(m_desc.m_attachments.begin(), m_desc.m_attachments.end(), 0,
            [](int accumulator, const ImageAttachment& imageAttachment)
            {
                return accumulator + (imageAttachment.m_viewAspectFlags & (ImageViewAspect_Depth | ImageViewAspect_Stencil)) ? 1 : 0;
            });
        DX_LOG(Info, "Vulkan FrameBuffer", "Frame buffer created with %d attachments. Color: %s (%d) DepthStencil: %s (%d)",
            m_imageViews.size(),
            (colorCount > 0) ? "YES" : "NO", colorCount,
            (depthStencilCount > 0) ? "YES" : "NO", depthStencilCount);

        return true;
    }
} // namespace Vulkan
