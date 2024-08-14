#pragma once

#include <array>

typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;

namespace Vulkan
{
    class Instance;

    enum VkQueueFamilyType
    {
        VkQueueFamilyType_Graphics = 0,
        VkQueueFamilyType_Compute,
        VkQueueFamilyType_Presentation,

        VkQueueFamilyType_Count,
    };

    // Manages the Vulkan physical device, logical device and queues.
    class Device
    {
    public:
        Device(Instance* instance, VkSurfaceKHR vkSurface);
        ~Device();

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        bool Initialize();
        void Terminate();

        VkDevice GetVkDevice();
        VkPhysicalDevice GetVkPhysicalDevice();
        VkSurfaceKHR GetVkSurface();

    private:
        Instance* m_instance = nullptr;
        VkSurfaceKHR m_vkSurface = nullptr;

    private:
        bool CreateVkDevice();

        VkPhysicalDevice m_vkPhysicalDevice = nullptr;
        VkDevice m_vkDevice = nullptr;
        std::array<VkQueue, VkQueueFamilyType_Count> m_vkQueues;
    };
} // namespace Vulkan
