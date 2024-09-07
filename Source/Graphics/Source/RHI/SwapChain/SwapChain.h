#pragma once

#include <RHI/Resource/ResourceEnums.h>

#include <Math/Vector2.h>

#include <vector>
#include <memory>

typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;

namespace Vulkan
{
    class Device;
    class Image;
    class FrameBuffer;
    class RenderPass;

    // Manages the Vulkan SwapChain
    class SwapChain
    {
    public:
        static bool CheckSwapChainSupported(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);

        SwapChain(Device* device);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        bool Initialize();
        void Terminate();

        uint32_t GetImageCount() const;
        ResourceFormat GetImageFormat() const;
        const Math::Vector2Int& GetImageSize() const;

        std::vector<std::shared_ptr<Image>> ObtainImagesFromSwapChain();

        VkSwapchainKHR GetVkSwapChain();

    private:
        Device* m_device = nullptr;

    private:
        bool CreateVkSwapChain();

        VkSwapchainKHR m_vkSwapChain = nullptr;

        uint32_t m_imageCount = 0;
        ResourceFormat m_imageFormat = ResourceFormat::Unknown;
        Math::Vector2Int m_imageSize = Math::Vector2Int(0);
    };
} // namespace Vulkan
