#include <RHI/SwapChain/SwapChain.h>

#include <RHI/Device/Instance.h>
#include <RHI/Device/Device.h>
#include <RHI/FrameBuffer/FrameBuffer.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

// Necessary to ask GLFW:
// - The frame buffer size
#define GLFW_INCLUDE_VULKAN // This will cause glfw3.h to include vulkan.h already
#include <GLFW/glfw3.h>

#include <algorithm>
#include <unordered_set>

namespace Vulkan
{
    namespace Utils
    {
        struct VkSwapChainInfo
        {
            VkSurfaceCapabilitiesKHR m_vkSurfaceCapabilities = {};
            std::vector<VkSurfaceFormatKHR> m_vkSupportedSurfaceFormats;
            std::vector<VkPresentModeKHR> m_vkSupportedPresentModes;
        };

        VkSwapChainInfo PopulateVkSwapChainInfo(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
        {
            VkSwapChainInfo vkSwapChainInfo;

            // Surface Capabilities
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                vkPhysicalDevice, vkSurface, &vkSwapChainInfo.m_vkSurfaceCapabilities);

            // Surface Formats
            vkSwapChainInfo.m_vkSupportedSurfaceFormats = [vkPhysicalDevice, vkSurface]()
                {
                    uint32_t formatCount = 0;
                    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, nullptr);

                    std::vector<VkSurfaceFormatKHR> vkSupportedSurfaceFormats(formatCount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, vkSupportedSurfaceFormats.data());

                    return vkSupportedSurfaceFormats;
                }();

            // Presentation Modes
            vkSwapChainInfo.m_vkSupportedPresentModes = [vkPhysicalDevice, vkSurface]()
                {
                    uint32_t presentCount = 0;
                    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentCount, nullptr);

                    std::vector<VkPresentModeKHR> vkSupportedPresentModes(presentCount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentCount, vkSupportedPresentModes.data());

                    return vkSupportedPresentModes;
                }();

            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(vkPhysicalDevice, &physicalDeviceProperties);
            DX_LOG(Verbose, "Vulkan SwapChain", "Vulkan Swap Chain Info by '%s':", physicalDeviceProperties.deviceName);
            DX_LOG(Verbose, "Vulkan SwapChain", "\t- Min image Count: %d", vkSwapChainInfo.m_vkSurfaceCapabilities.minImageCount);
            DX_LOG(Verbose, "Vulkan SwapChain", "\t- Max image Count: %d", vkSwapChainInfo.m_vkSurfaceCapabilities.maxImageCount);
            DX_LOG(Verbose, "Vulkan SwapChain", "\t- Current image size: %dx%d",
                vkSwapChainInfo.m_vkSurfaceCapabilities.currentExtent.width,
                vkSwapChainInfo.m_vkSurfaceCapabilities.currentExtent.height);
            DX_LOG(Verbose, "Vulkan SwapChain", "\t- Min image size: %dx%d",
                vkSwapChainInfo.m_vkSurfaceCapabilities.minImageExtent.width,
                vkSwapChainInfo.m_vkSurfaceCapabilities.minImageExtent.height);
            DX_LOG(Verbose, "Vulkan SwapChain", "\t- Max image size: %dx%d",
                vkSwapChainInfo.m_vkSurfaceCapabilities.maxImageExtent.width,
                vkSwapChainInfo.m_vkSurfaceCapabilities.maxImageExtent.height);
            DX_LOG(Verbose, "Vulkan SwapChain", "\t- Surface formats: %d", vkSwapChainInfo.m_vkSupportedSurfaceFormats.size());
            DX_LOG(Verbose, "Vulkan SwapChain", "\t- Presentation modes: %d", vkSwapChainInfo.m_vkSupportedPresentModes.size());

            return vkSwapChainInfo;
        }

        bool AllSurfaceFormatsSupported(const std::vector<VkSurfaceFormatKHR>& vkSurfaceFormats)
        {
            return vkSurfaceFormats.size() == 1 && vkSurfaceFormats[0].format == VK_FORMAT_UNDEFINED;
        }

        bool AnyImageSizeAllowed(const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities)
        {
            return vkSurfaceCapabilities.maxImageCount <= 0;
        }

        // Best surface format is subjective, but ours will be:
        // Format: VK_FORMAT_R8G8B8A8_UNORM or VK_FORMAT_B8G8R8A8_UNORM
        // ColorSpace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& vkSurfaceFormats)
        {
            if (vkSurfaceFormats.empty())
            {
                return {};
            }
            else if (AllSurfaceFormatsSupported(vkSurfaceFormats))
            {
                return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
            }

            // Search for preferred surface format
            auto surfaceFormatIt = std::find_if(vkSurfaceFormats.begin(), vkSurfaceFormats.end(),
                [](const VkSurfaceFormatKHR& vkSurfaceFormat)
                {
                    return (vkSurfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM || vkSurfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
                        && vkSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                });

            // Found it?
            if (surfaceFormatIt != vkSurfaceFormats.end())
            {
                return *surfaceFormatIt;
            }
            // Else return first one from the list.
            else
            {
                return vkSurfaceFormats[0];
            }
        }

        // Best present mode is subjective, but ours will be VK_PRESENT_MODE_MAILBOX_KHR
        VkPresentModeKHR ChooseBestPresentMode(const std::vector<VkPresentModeKHR>& vkPresentModes)
        {
            // Search for preferred present mode
            auto presentModeIt = std::find(vkPresentModes.begin(), vkPresentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR);

            // Found it?
            if (presentModeIt != vkPresentModes.end())
            {
                return *presentModeIt;
            }
            // Else return VK_PRESENT_MODE_FIFO_KHR as it should always be available according to Vulkan Spec.
            else
            {
                return VK_PRESENT_MODE_FIFO_KHR;
            }
        }

        VkExtent2D ObtainSwapChainImageExtent(
            const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities,
            GLFWwindow* windowHandler)
        {
            // If current extent is at numeric limits, that means the extent can vary.
            // In this case use the frame buffer size.
            if (vkSurfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
            {
                // Get frame buffer size in pixels from GLWF
                int width = 0, height = 0;
                glfwGetFramebufferSize(windowHandler, &width, &height);

                // Make sure size stays within surface max/min image extent values.
                Math::Vector2Int clampedFrameBufferSize = mathfu::Clamp(
                    Math::Vector2Int(width, height),
                    Math::Vector2Int(vkSurfaceCapabilities.minImageExtent.width, vkSurfaceCapabilities.minImageExtent.height),
                    Math::Vector2Int(vkSurfaceCapabilities.maxImageExtent.width, vkSurfaceCapabilities.maxImageExtent.height));

                return { static_cast<uint32_t>(clampedFrameBufferSize.x), static_cast<uint32_t>(clampedFrameBufferSize.y) };
            }
            // Otherwise the current extent is the size of the window.
            else
            {
                return vkSurfaceCapabilities.currentExtent;
            }
        }
    } // namespace Utils

    bool SwapChain::CheckSwapChainSupported(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
    {
        const Utils::VkSwapChainInfo vkSwapChainInfo = Utils::PopulateVkSwapChainInfo(vkPhysicalDevice, vkSurface);

        return !vkSwapChainInfo.m_vkSupportedSurfaceFormats.empty()
            && !vkSwapChainInfo.m_vkSupportedPresentModes.empty();
    }

    SwapChain::SwapChain(Device* device)
        : m_device(device)
    {
    }

    SwapChain::~SwapChain()
    {
        Terminate();
    }

    bool SwapChain::Initialize()
    {
        if (m_vkSwapChain)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan SwapChain", "Initializing Vulkan SwapChain...");

        if (!CreateVkSwapChain())
        {
            Terminate();
            return false;
        }

        DX_ASSERT(MaxFrameDraws < m_imageCount, "Vulkan SwapChain",
            "MaxFrameDraws (%d) is greater or equal than swap chain's images (%d).",
            MaxFrameDraws, m_imageCount);

        return true;
    }

    void SwapChain::Terminate()
    {
        DX_LOG(Info, "Vulkan SwapChain", "Terminating Vulkan SwapChain...");

        vkDestroySwapchainKHR(m_device->GetVkDevice(), m_vkSwapChain, nullptr);
        m_vkSwapChain = nullptr;
    }

    uint32_t SwapChain::GetImageCount() const
    {
        return m_imageCount;
    }

    ResourceFormat SwapChain::GetImageFormat() const
    {
        return m_imageFormat;
    }

    const Math::Vector2Int& SwapChain::GetImageSize() const
    {
        return m_imageSize;
    }

    VkSwapchainKHR SwapChain::GetVkSwapChain()
    {
        return m_vkSwapChain;
    }

    bool SwapChain::CreateVkSwapChain()
    {
        const Utils::VkSwapChainInfo vkSwapChainInfo = 
            Utils::PopulateVkSwapChainInfo(m_device->GetVkPhysicalDevice(), m_device->GetInstance()->GetVkSurface());

        // Find optimal surface values for our swap chain.
        const VkSurfaceFormatKHR vkSurfaceFormat = Utils::ChooseBestSurfaceFormat(vkSwapChainInfo.m_vkSupportedSurfaceFormats);
        const VkPresentModeKHR vkPresentMode = Utils::ChooseBestPresentMode(vkSwapChainInfo.m_vkSupportedPresentModes);
        const VkExtent2D vkImageExtent = Utils::ObtainSwapChainImageExtent(vkSwapChainInfo.m_vkSurfaceCapabilities, m_device->GetInstance()->GetWindowHandler());

        // Number of images in the swap chain.
        // Use 1 more than the minimum to allow triple buffering.
        const uint32_t targetImageCount = [&vkSwapChainInfo]()
            {
                if (Utils::AnyImageSizeAllowed(vkSwapChainInfo.m_vkSurfaceCapabilities))
                {
                    return vkSwapChainInfo.m_vkSurfaceCapabilities.minImageCount + 1;
                }
                else
                {
                    return std::min(
                        vkSwapChainInfo.m_vkSurfaceCapabilities.minImageCount + 1,
                        vkSwapChainInfo.m_vkSurfaceCapabilities.maxImageCount);
                }
            }(); 

        // If queue families use different queues, then swap chain must let
        // its images be shared between families.
        const std::vector<uint32_t>& uniqueFamilyIndices = m_device->GetQueueFamilyInfo().m_uniqueQueueFamilyIndices;

        VkSwapchainCreateInfoKHR vkSwapchainCreateInfo = {};
        vkSwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        vkSwapchainCreateInfo.pNext = nullptr;
        vkSwapchainCreateInfo.flags = 0;
        vkSwapchainCreateInfo.surface = m_device->GetInstance()->GetVkSurface();
        vkSwapchainCreateInfo.minImageCount = targetImageCount;
        vkSwapchainCreateInfo.imageFormat = vkSurfaceFormat.format;
        vkSwapchainCreateInfo.imageColorSpace = vkSurfaceFormat.colorSpace;
        vkSwapchainCreateInfo.imageExtent = vkImageExtent;
        vkSwapchainCreateInfo.imageArrayLayers = 1;
        vkSwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (uniqueFamilyIndices.size() > 1)
        {
            vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            vkSwapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(uniqueFamilyIndices.size());
            vkSwapchainCreateInfo.pQueueFamilyIndices = uniqueFamilyIndices.data();
        }
        else
        {
            vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            vkSwapchainCreateInfo.queueFamilyIndexCount = 0;
            vkSwapchainCreateInfo.pQueueFamilyIndices = nullptr;
        }
        vkSwapchainCreateInfo.preTransform = vkSwapChainInfo.m_vkSurfaceCapabilities.currentTransform;
        vkSwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        vkSwapchainCreateInfo.presentMode = vkPresentMode;
        vkSwapchainCreateInfo.clipped = VK_TRUE; // Clip parts of images not in view (e.g. behind another window)
        vkSwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // Can be useful when resizing window when recreating swap chain

        if (vkCreateSwapchainKHR(m_device->GetVkDevice(), &vkSwapchainCreateInfo, nullptr, &m_vkSwapChain) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan SwapChain", "Failed to create Vulkan SwapChain.");
            return false;
        }

        // Store recurrent swap chain properties
        m_imageFormat = ToResourceFormat(vkSurfaceFormat.format);
        m_imageSize = Math::Vector2Int(vkImageExtent.width, vkImageExtent.height);
        vkGetSwapchainImagesKHR(m_device->GetVkDevice(), m_vkSwapChain, &m_imageCount, nullptr);

        DX_LOG(Verbose, "Vulkan SwapChain", "SwapChain Properties:");
        DX_LOG(Verbose, "Vulkan SwapChain", "\t- Image Size: %dx%d", m_imageSize.x, m_imageSize.y);
        DX_LOG(Verbose, "Vulkan SwapChain", "\t- Image Count: %d", m_imageCount);
        DX_LOG(Verbose, "Vulkan SwapChain", "\t- Image Format: %d", m_imageFormat);
        DX_LOG(Verbose, "Vulkan SwapChain", "\t- Image Color Space: %d", vkSurfaceFormat.colorSpace);
        DX_LOG(Verbose, "Vulkan SwapChain", "\t- Present Mode: %d", vkPresentMode);
        DX_LOG(Verbose, "Vulkan SwapChain", "\t- Unique Queue Family Indices: %d", uniqueFamilyIndices.size());

        return true;
    }

    std::vector<std::shared_ptr<Image>> SwapChain::ObtainImagesFromSwapChain()
    {
        // Obtain the images that have been created as part of the swap chain.
        // The vulkan images are obtained from the swap chain.
        std::vector<VkImage> vkSwapChainImages(m_imageCount);
        vkGetSwapchainImagesKHR(m_device->GetVkDevice(), m_vkSwapChain, &m_imageCount, vkSwapChainImages.data());

        if (std::any_of(vkSwapChainImages.begin(), vkSwapChainImages.end(),
            [](const VkImage& vkImage)
            {
                return vkImage == nullptr;
            }))
        {
            return {};
        }

        std::vector<std::shared_ptr<Image>> swapChainImages;
        swapChainImages.reserve(vkSwapChainImages.size());

        for (auto& vkSwapChainImage : vkSwapChainImages)
        {
            ImageDesc imageDesc = {};
            imageDesc.m_imageType = ImageType::Image2D;
            imageDesc.m_dimensions = Math::Vector3Int(m_imageSize, 1);
            imageDesc.m_mipCount = 1;
            imageDesc.m_format = m_imageFormat;
            imageDesc.m_tiling = ImageTiling::Optimal;
            imageDesc.m_usageFlags = ImageUsage_ColorAttachment;
            imageDesc.m_nativeResource = ImageDesc::NativeResource {
               .m_imageNativeResource = vkSwapChainImage,
               .m_imageMemoryNativeResource = nullptr,
               .m_ownsNativeResource = false
            };

            auto image = std::make_shared<Image>(m_device, imageDesc);
            if (!image->Initialize())
            {
                return {};
            }

            swapChainImages.push_back(std::move(image));
        }

        return swapChainImages;
    }
} // namespace Vulkan
