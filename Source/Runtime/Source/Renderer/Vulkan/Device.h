#pragma once

#include <array>

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

    // Maps family type to Vulkan queue family index (in the physical device)
    // Different family types might use the same queue family index.
    // Useful to query which is the family index of a type.
    struct VkQueueFamilyInfo
    {
        std::array<int, VkQueueFamilyType_Count> m_familyTypeToFamilyIndices;

        VkQueueFamilyInfo()
        {
            // Start with invalid indices
            m_familyTypeToFamilyIndices.fill(-1);
        }

        // Check if all families have been found
        bool IsValid() const;
    };

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

        Instance* GetInstance();
        VkDevice GetVkDevice();
        VkPhysicalDevice GetVkPhysicalDevice();

        const VkQueueFamilyInfo& GetVkQueueFamilyInfo() const;

    private:
        Instance* m_instance = nullptr;

    private:
        bool CreateVkDevice();

        VkPhysicalDevice m_vkPhysicalDevice = nullptr;
        VkQueueFamilyInfo m_vkQueueFamilyInfo;
        VkDevice m_vkDevice = nullptr;
        std::array<VkQueue, VkQueueFamilyType_Count> m_vkQueues;
    };
} // namespace Vulkan
