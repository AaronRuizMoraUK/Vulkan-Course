#include <RHI/Resource/Image/Image.h>

#include <RHI/Device/Device.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    namespace Utils
    {
        bool CreateVkImage(
            Device* device,
            VkImageType vkImageType,
            const Math::Vector3Int& dimensions,
            uint32_t mipCount,
            VkFormat vkFormat,
            VkImageTiling vkImageTiling,
            VkImageUsageFlags vkImageUsageFlags,
            VkMemoryPropertyFlags vkMemoryPropertyFlags,
            VkImage* vkImageOut,
            VkDeviceMemory* vkImageMemoryOut)
        {
            // Create Image object
            {
                // If queue families use different queues, then image must be shared between families.
                const std::vector<uint32_t>& uniqueFamilyIndices = device->GetQueueFamilyInfo().m_uniqueQueueFamilyIndices;

                VkImageCreateInfo vkImageCreateInfo = {};
                vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                vkImageCreateInfo.pNext = nullptr;
                vkImageCreateInfo.flags = 0;
                vkImageCreateInfo.imageType = vkImageType;
                vkImageCreateInfo.format = vkFormat;
                vkImageCreateInfo.extent.width = dimensions.x;
                vkImageCreateInfo.extent.height = dimensions.y;
                vkImageCreateInfo.extent.depth = dimensions.z; // Must be greater than 0
                vkImageCreateInfo.mipLevels = mipCount;
                vkImageCreateInfo.arrayLayers = 1;
                vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT; // Number of samples for multi-sampling
                vkImageCreateInfo.tiling = vkImageTiling;
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
                vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Layout of image data on creation

                if (vkCreateImage(device->GetVkDevice(), &vkImageCreateInfo, nullptr, vkImageOut) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Image", "Failed to create Vulkan Image.");
                    return false;
                }
            }

            // Allocate memory for image and link them together
            {
                // Get Image's Memory Requirements
                VkMemoryRequirements vkMemoryRequirements = {};
                vkGetImageMemoryRequirements(device->GetVkDevice(), *vkImageOut, &vkMemoryRequirements);

                // Allocate memory for the image
                VkMemoryAllocateInfo vkMemoryAllocateInfo = {};
                vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                vkMemoryAllocateInfo.pNext = nullptr;
                vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
                vkMemoryAllocateInfo.memoryTypeIndex = FindCompatibleMemoryTypeIndex(
                    device->GetVkPhysicalDevice(), vkMemoryRequirements.memoryTypeBits, vkMemoryPropertyFlags);

                if (vkAllocateMemory(device->GetVkDevice(), &vkMemoryAllocateInfo, nullptr, vkImageMemoryOut) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Image", "Failed to allocate memory for Vulkan Image.");
                    return false;
                }

                // Link the image to the memory
                if (vkBindImageMemory(device->GetVkDevice(), *vkImageOut, *vkImageMemoryOut, 0) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Image", "Failed to bind Vulkan image to memory.");
                    return false;
                }
            }

            return true;
        }

        void DestroyVkImage(Device* device, VkImage& vkImage, VkDeviceMemory& vkImageMemory)
        {
            vkDestroyImage(device->GetVkDevice(), vkImage, nullptr);
            vkImage = nullptr;

            vkFreeMemory(device->GetVkDevice(), vkImageMemory, nullptr);
            vkImageMemory = nullptr;
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

        if (m_vkImage && !m_desc.m_nativeResource.has_value())
        {
            DX_LOG(Verbose, "Vulkan Image", "Image %s %dx%dx%d and %d mipmaps destroyed.",
                ImageTypeStr(m_desc.m_imageType), m_desc.m_dimensions.x, m_desc.m_dimensions.y, m_desc.m_dimensions.z, m_desc.m_mipCount);
        }

        const bool skipDestruction = 
            m_desc.m_nativeResource.has_value() && 
            !m_desc.m_nativeResource->m_ownsNativeResource;

        if (skipDestruction)
        {
            m_vkImage = nullptr;
            m_vkImageMemory = nullptr;
        }
        else
        {
            Utils::DestroyVkImage(m_device, m_vkImage, m_vkImageMemory);
        }
    }

    VkImage Image::GetVkImage()
    {
        return m_vkImage;
    }

    bool Image::CreateVkImage()
    {
        if (m_desc.m_usageFlags == 0)
        {
            DX_LOG(Error, "Vulkan Image", "Image description with no usage flag set.");
            return false;
        }

        if (m_desc.m_nativeResource.has_value())
        {
            if (!m_desc.m_nativeResource->m_imageNativeResource)
            {
                DX_LOG(Error, "Vulkan Image", "Image description with invalid data.");
                return false;
            }
            else if (m_desc.m_nativeResource->m_ownsNativeResource && 
                !m_desc.m_nativeResource->m_imageMemoryNativeResource)
            {
                DX_LOG(Error, "Vulkan Image",
                    "Indicated that the image should own the resources but image memory was not provided.");
                return false;
            }

            m_vkImage = static_cast<VkImage>(m_desc.m_nativeResource->m_imageNativeResource);
            m_vkImageMemory = static_cast<VkDeviceMemory>(m_desc.m_nativeResource->m_imageMemoryNativeResource);

            // NOTE: Expected that the VkImage is already linked to the VkDeviceMemory.

            // When native resource are directly provided, the initial data will be ignored since
            // there is no enough information about the image and its memory to perform a copy.
            if (m_desc.m_initialData != nullptr)
            {
                DX_LOG(Warning, "Vulkan Image", 
                    "Initial data provided will be ignored since the image native resources was directly provided.");
            }
            return true;
        }

        const VkImageUsageFlags vkImageUsageFlags = ToVkImageUsageFlags(m_desc.m_usageFlags);

        const VkMemoryPropertyFlags vkMemoryProperties = [this]() -> int
            {
                switch (m_desc.m_memoryProperty)
                {
                case ResourceMemoryProperty::HostVisible:
                    return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

                case ResourceMemoryProperty::DeviceLocal:
                    return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

                default:
                    DX_LOG(Fatal, "Vulkan Image", "Unexpected resource memory property %d.", m_desc.m_memoryProperty);
                    return 0;
                }
            }();

        if (!Utils::CreateVkImage(m_device, 
            ToVkImageType(m_desc.m_imageType),
            m_desc.m_dimensions,
            m_desc.m_mipCount,
            ToVkFormat(m_desc.m_format),
            ToVkImageTiling(m_desc.m_tiling),
            vkImageUsageFlags,
            vkMemoryProperties,
            &m_vkImage,
            &m_vkImageMemory))
        {
            return false;
        }

        // TODO: copy initial data handling the different possible m_desc.m_memoryProperty

        DX_LOG(Verbose, "Vulkan Image", "Image %s %dx%dx%d and %d mipmaps created.",
            ImageTypeStr(m_desc.m_imageType), m_desc.m_dimensions.x, m_desc.m_dimensions.y, m_desc.m_dimensions.z, m_desc.m_mipCount);

        return true;
    }
} // namespace Vulkan
