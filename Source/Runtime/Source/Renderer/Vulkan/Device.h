#pragma once

#include <Renderer/Vulkan/Instance.h>

#include <vector>

typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;

namespace Vulkan
{
    class Instance;

    // Manages the Vulkan physical device, logical device and queues.
    class Device
    {
    public:
        Device(Instance* instance);
        ~Device();

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        bool Initialize();
        void Terminate();

        VkDevice GetVkDevice();

    private:
        // Location of Queue Families in a Vulkan physical device
        struct VkQueueFamilyIndices
        {
            int m_graphicsFamily = -1;

            // Check if all families have been found
            bool IsValid() const
            {
                return m_graphicsFamily >= 0;
            }
        };

        bool CheckVkPhysicalDeviceSuitable(VkPhysicalDevice vkPhysicalDevice) const;
        VkQueueFamilyIndices EnumerateVkQueueFamilies(VkPhysicalDevice vkPhysicalDevice) const;

        bool CreateVkLogicalDevice();

        Instance* m_instance = nullptr;
        VkPhysicalDevice m_vkPhysicalDevice = nullptr;
        VkQueueFamilyIndices m_vkQueueFamilyIndices;
        VkDevice m_vkDevice = nullptr;
        VkQueue m_vkGraphicsQueue = nullptr;
    };
} // namespace Vulkan
