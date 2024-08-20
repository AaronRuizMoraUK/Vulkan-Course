#pragma once

#include <array>
#include <vector>

typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;
typedef struct VkCommandPool_T* VkCommandPool;

namespace Vulkan
{
    class Instance;

    enum QueueFamilyType
    {
        QueueFamilyType_Graphics = 0,
        QueueFamilyType_Compute,
        QueueFamilyType_Presentation,

        QueueFamilyType_Count,
    };

    struct QueueFamilyInfo
    {
        // Maps family type to Vulkan queue family index (in the physical device)
        // Different family types might use the same queue family index.
        // Useful to query which is the family index of a type.
        std::array<int, QueueFamilyType_Count> m_familyTypeToFamilyIndices;

        // List of unique queue family indices.
        std::vector<uint32_t> m_uniqueQueueFamilyIndices;

        QueueFamilyInfo()
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
        VkQueue GetVkQueue(QueueFamilyType queueFamilyType);
        VkCommandPool GetVkCommandPool(QueueFamilyType queueFamilyType);

        const QueueFamilyInfo& GetQueueFamilyInfo() const;

    private:
        Instance* m_instance = nullptr;

    private:
        bool CreateVkDevice();
        bool CreateVkCommandPools();

        VkPhysicalDevice m_vkPhysicalDevice = nullptr;
        QueueFamilyInfo m_queueFamilyInfo;
        VkDevice m_vkDevice = nullptr;
        std::array<VkQueue, QueueFamilyType_Count> m_vkQueues;
        std::array<VkCommandPool, QueueFamilyType_Count> m_vkCommandPools;
    };
} // namespace Vulkan
