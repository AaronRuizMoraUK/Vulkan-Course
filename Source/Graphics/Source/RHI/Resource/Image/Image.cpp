#include <RHI/Resource/Image/Image.h>

#include <RHI/Device/Device.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    namespace Utils
    {
        bool CreateVkImage(
            VkDevice vkDevice,
            const std::vector<uint32_t>& uniqueFamilyIndices,
            const Math::Vector3Int& size,
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
            vkImageCreateInfo.extent.depth = size.z;
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
    } // Utils

    Image::Image(Device* device, const ImageDesc& desc)
        : m_device(device)
        , m_desc(desc)
    {
    }

    Image::~Image()
    {
        Terminate();
    }

    bool Image::Initialize()
    {
        if (m_vkImage)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan Image", "Initializing Vulkan Image...");

        if (!CreateVkImage())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Image::Terminate()
    {
        DX_LOG(Info, "Vulkan Image", "Terminating Vulkan Image...");

        const bool skipDestruction = m_desc.m_initialDataIsNativeResource && !m_desc.m_ownInitialNativeResource;
        if (!skipDestruction)
        {
            vkDestroyImage(m_device->GetVkDevice(), m_vkImage, nullptr);
        }
        m_vkImage = nullptr;
    }

    VkImage Image::GetVkImage()
    {
        return m_vkImage;
    }

    bool Image::CreateVkImage()
    {
        if (m_desc.m_initialDataIsNativeResource)
        {
            if (!m_desc.m_initialData)
            {
                DX_LOG(Fatal, "Vulkan Image", "Image description with invalid data.");
                return false;
            }

            m_vkImage = static_cast<VkImage>(const_cast<void*>(m_desc.m_initialData));
        }
        else
        {
            // TODO
            DX_ASSERT(false, "Vulkan Image", "Not implemented!");
            return false;
        }

        return true;
    }
} // namespace Vulkan
