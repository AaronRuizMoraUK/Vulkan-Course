#pragma once

#include <Math/Vector2.h>

#include <vector>
#include <memory>

typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;
typedef struct VkRenderPass_T* VkRenderPass;

namespace Vulkan
{
    class Device;
    class FrameBuffer;

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

        bool CreateFrameBuffers(VkRenderPass vkRenderPass);
        void DestroyFrameBuffers();

        int GetImageFormat() const;
        const Math::Vector2Int& GetImageSize() const;

        uint32_t GetImageCount() const;
        FrameBuffer* GetFrameBuffer(uint32_t imageIndex);

        VkSwapchainKHR GetVkSwapChain();

    private:
        Device* m_device = nullptr;

    private:
        bool CreateVkSwapChain();

        VkSwapchainKHR m_vkSwapChain = nullptr;

        uint32_t m_imageCount = 0;
        int m_imageFormat = -1;
        Math::Vector2Int m_imageSize = Math::Vector2Int(0);

        // Frame buffers for drawing into each swap chain image.
        std::vector<std::unique_ptr<FrameBuffer>> m_frameBuffers;
    };
} // namespace Vulkan
