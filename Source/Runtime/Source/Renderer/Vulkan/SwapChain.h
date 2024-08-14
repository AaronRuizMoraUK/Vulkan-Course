#pragma once

#include <Math/Vector2.h>

typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;

namespace Vulkan
{
    class Device;

    // Manages the Vulkan SwapChain
    class SwapChain
    {
    public:
        static bool CheckSwapChainSupported(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);

        SwapChain(Device* device, const Math::Vector2Int& defaultFrameBufferSize);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        bool Initialize();
        void Terminate();

    private:
        Device* m_device = nullptr;
        const Math::Vector2Int m_defaultFrameBufferSize;

        int m_imageFormat = -1;
        Math::Vector2Int m_imageSize = Math::Vector2Int(0);

    private:
        bool CreateVkSwapChain();

        VkSwapchainKHR m_vkSwapChain = nullptr;
    };
} // namespace Vulkan
