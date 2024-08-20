#include <Renderer/Vulkan/SwapChain.h>

#include <Renderer/Vulkan/Instance.h>
#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/FrameBuffer.h>
#include <Renderer/Vulkan/CommandBuffer.h>

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

        // Best format is subjective, but ours will be:
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

        if (!CreateCommandBuffers())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void SwapChain::Terminate()
    {
        DX_LOG(Info, "Vulkan SwapChain", "Terminating Vulkan SwapChain...");

        DestroyFrameBuffers();

        m_commandBuffers.clear();

        vkDestroySwapchainKHR(m_device->GetVkDevice(), m_vkSwapChain, nullptr);
        m_vkSwapChain = nullptr;
    }

    int SwapChain::GetImageFormat() const
    {
        return m_imageFormat;
    }

    const Math::Vector2Int& SwapChain::GetImageSize() const
    {
        return m_imageSize;
    }

    uint32_t SwapChain::GetImageCount() const
    {
        return m_imageCount;
    }

    FrameBuffer* SwapChain::GetFrameBuffer(uint32_t imageIndex)
    {
        return (imageIndex < m_frameBuffers.size())
            ? m_frameBuffers[imageIndex].get()
            : nullptr;
    }

    CommandBuffer* SwapChain::GetCommandBuffer(uint32_t imageIndex)
    {
        return (imageIndex < m_commandBuffers.size())
            ? m_commandBuffers[imageIndex].get()
            : nullptr;
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
        m_imageFormat = vkSurfaceFormat.format;
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

    bool SwapChain::CreateCommandBuffers()
    {
        // Create graphics command buffers for all frame buffers of the swap chain.
        // That's one command buffer for each image of the swap chain.
        m_commandBuffers.resize(m_imageCount);
        for (auto& commandBuffer : m_commandBuffers)
        {
            commandBuffer = std::make_unique<CommandBuffer>(m_device, m_device->GetVkCommandPool(QueueFamilyType_Graphics));

            if (!commandBuffer->Initialize())
            {
                DX_LOG(Error, "Vulkan SwapChain", "Failed to create CommandBuffer.");
                return false;
            }
        }

        return true;
    }

    bool SwapChain::CreateFrameBuffers(VkRenderPass vkRenderPass)
    {
        DX_LOG(Info, "Vulkan SwapChain", "Creating Vulkan FrameBuffers for SwapChain...");

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
            DX_LOG(Error, "Vulkan SwapChain", "Failed to populate Vulkan SwapChain Images.");
            return false;
        }

        std::vector<std::unique_ptr<FrameBuffer>> frameBuffers;
        frameBuffers.reserve(vkSwapChainImages.size());

        for (VkImage vkSwapChainImage : vkSwapChainImages)
        {
            const Image colorImage = {
                .m_vkImage = vkSwapChainImage,
                .m_vkFormat = m_imageFormat,
                .m_size = m_imageSize
            };
            const bool createDepthAttachment = false;

            auto frameBuffer = std::make_unique<FrameBuffer>(m_device, vkRenderPass, colorImage);

            if (!frameBuffer->Initialize(createDepthAttachment))
            {
                DX_LOG(Error, "Vulkan SwapChain", "Failed to create FrameBuffer.");
                return false;
            }

            frameBuffers.push_back(std::move(frameBuffer));
        }

        m_frameBuffers = std::move(frameBuffers);

        return true;
    }

    void SwapChain::DestroyFrameBuffers()
    {
        DX_LOG(Info, "Vulkan SwapChain", "Destroying Vulkan FrameBuffers from SwapChain...");

        m_frameBuffers.clear();
    }
} // namespace Vulkan
