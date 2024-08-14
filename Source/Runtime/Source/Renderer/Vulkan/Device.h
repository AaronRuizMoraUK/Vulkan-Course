#pragma once

#include <Renderer/Vulkan/Instance.h>

#include <array>
#include <vector>

typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
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
        Device(Instance* instance, VkSurfaceKHR vkSurface);
        ~Device();

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        bool Initialize();
        void Terminate();

        VkDevice GetVkDevice();

    private:
        Instance* m_instance = nullptr;
        VkSurfaceKHR m_vkSurface = nullptr;

        enum VkQueueFamilyType
        {
            VkQueueFamilyType_Graphics = 0,
            VkQueueFamilyType_Compute,
            VkQueueFamilyType_Presentation,

            VkQueueFamilyType_Count,
        };

        struct VkQueueFamilyInfo
        {
            // Maps family type to Vulkan queue family index (in the physical device)
            // Different family types might use the same queue family index.
            // Useful to query which is the family index of a type.
            std::array<int, VkQueueFamilyType_Count> m_familyTypeToFamilyIndices;

            VkQueueFamilyInfo()
            {
                // Start with invalid indices
                m_familyTypeToFamilyIndices.fill(-1);
            }

            // Check if all families have been found
            bool IsValid() const;
        };

        bool CheckVkPhysicalDeviceSuitable(VkPhysicalDevice vkPhysicalDevice, const std::vector<const char*>& extensions) const;
        bool VkDeviceExtensionsSupported(VkPhysicalDevice vkPhysicalDevice, const std::vector<const char*>& extensions) const;
        VkQueueFamilyInfo EnumerateVkQueueFamilies(VkPhysicalDevice vkPhysicalDevice) const;

        bool CreateVkLogicalDevice();

        VkPhysicalDevice m_vkPhysicalDevice = nullptr;
        VkQueueFamilyInfo m_vkQueueFamilyInfo;
        VkDevice m_vkDevice = nullptr;
        std::array<VkQueue, VkQueueFamilyType_Count> m_vkQueues;
    };
} // namespace Vulkan
