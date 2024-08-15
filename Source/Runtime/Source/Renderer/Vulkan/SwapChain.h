#pragma once

#include <Math/Vector2.h>

#include <vector>

typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;
typedef struct VkImage_T* VkImage;
typedef struct VkImageView_T* VkImageView;

namespace Vulkan
{
    class Device;

    struct SwapChainImage
    {
        VkImage m_vkImage = nullptr;
        VkImageView m_vkImageView = nullptr;
    };

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

    private:
        Device* m_device = nullptr;

    private:
        bool CreateVkSwapChain();

        VkSwapchainKHR m_vkSwapChain = nullptr;

        int m_imageFormat = -1;
        Math::Vector2Int m_imageSize = Math::Vector2Int(0);
        std::vector<SwapChainImage> m_swapChainImages;
    };
} // namespace Vulkan
