#include <RHI/Resource/ImageView/ImageView.h>

#include <RHI/Device/Device.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    ImageView::ImageView(Device* device, const ImageViewDesc& desc)
        : m_device(device)
        , m_desc(desc)
    {
    }

    ImageView::~ImageView()
    {
        Terminate();
    }

    bool ImageView::Initialize()
    {
        if (m_vkImageView)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan ImageView", "Initializing Vulkan ImageView...");

        if (!CreateVkImageView())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void ImageView::Terminate()
    {
        DX_LOG(Info, "Vulkan ImageView", "Terminating Vulkan ImageView...");

        vkDestroyImageView(m_device->GetVkDevice(), m_vkImageView, nullptr);
        m_vkImageView = nullptr;
    }

    VkImageView ImageView::GetVkImageView()
    {
        return m_vkImageView;
    }

    bool ImageView::CreateVkImageView()
    {
        VkImageViewCreateInfo vkImageViewCreateInfo = {};
        vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vkImageViewCreateInfo.pNext = nullptr;
        vkImageViewCreateInfo.flags = 0;
        vkImageViewCreateInfo.image = m_desc.m_image->GetVkImage();
        vkImageViewCreateInfo.viewType = ToVkImageViewType(m_desc.m_image->GetImageDesc().m_imageType);
        vkImageViewCreateInfo.format = ToVkFormat(m_desc.m_viewFormat);
        vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // With sub-resource range it specifies the part of the image to view
        vkImageViewCreateInfo.subresourceRange.aspectMask = ToVkImageAspectFlags(m_desc.m_aspectFlags);
        vkImageViewCreateInfo.subresourceRange.baseMipLevel = m_desc.m_firstMip;
        vkImageViewCreateInfo.subresourceRange.levelCount = (m_desc.m_mipCount == 0) ? VK_REMAINING_MIP_LEVELS : m_desc.m_mipCount;
        vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        vkImageViewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        if (vkCreateImageView(m_device->GetVkDevice(), &vkImageViewCreateInfo, nullptr, &m_vkImageView) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan ImageView", "Failed to create Vulkan ImageView.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
