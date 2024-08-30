#include <Renderer/Vulkan/Device.h>

#include <Renderer/Vulkan/Instance.h>
#include <Renderer/Vulkan/SwapChain.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <unordered_set>
#include <algorithm>
#include <span>

namespace Vulkan
{
    // Vulkan device extensions that physical device must support
    static const std::array<const char* const, 1> VkDeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // Utils to extract information and perform checks on Vulkan Physical Devices
    namespace Utils
    {
        QueueFamilyInfo EnumerateQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
        {
            // Queue family properties of a Vulkan physical devices
            const std::vector<VkQueueFamilyProperties> queueFamilyProperties = [vkPhysicalDevice]()
                {
                    uint32_t queueFamilyCount = 0;
                    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);

                    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
                    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

                    return queueFamilyProperties;
                }();

            QueueFamilyInfo queueFamilyInfo;
            for (int queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size() && !queueFamilyInfo.IsValid(); ++queueFamilyIndex)
            {
                // ------------------
                // Check Graphics
                const bool queueFamilySupportsGraphics =
                    queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
                    queueFamilyProperties[queueFamilyIndex].queueCount > 0;

                // If Graphics is supported and it's the first queue family found for it, assign it.
                if (queueFamilyInfo.m_familyTypeToFamilyIndices[QueueFamilyType_Graphics] < 0 &&
                    queueFamilySupportsGraphics)
                {
                    queueFamilyInfo.m_familyTypeToFamilyIndices[QueueFamilyType_Graphics] = queueFamilyIndex;
                }

                // ------------------
                // Check Compute
                const bool queueFamilySupportsCompute =
                    queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT &&
                    queueFamilyProperties[queueFamilyIndex].queueCount > 0;

                // If Compute is supported and it's the first queue family found for it, assign it.
                if (queueFamilyInfo.m_familyTypeToFamilyIndices[QueueFamilyType_Compute] < 0 &&
                    queueFamilySupportsCompute)
                {
                    queueFamilyInfo.m_familyTypeToFamilyIndices[QueueFamilyType_Compute] = queueFamilyIndex;
                }

                // ------------------
                // Check Presentation
                VkBool32 vkQueueFamilySupportsSurface = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, queueFamilyIndex, vkSurface, &vkQueueFamilySupportsSurface);

                const bool queueFamilySupportsPresentation =
                    vkQueueFamilySupportsSurface &&
                    queueFamilyProperties[queueFamilyIndex].queueCount > 0;

                // If Presentation is supported and it's the first queue family found for it, assign it.
                if (queueFamilyInfo.m_familyTypeToFamilyIndices[QueueFamilyType_Presentation] < 0 &&
                    queueFamilySupportsPresentation)
                {
                    queueFamilyInfo.m_familyTypeToFamilyIndices[QueueFamilyType_Presentation] = queueFamilyIndex;
                }
            }

            // Make the list of unique family indices
            const std::unordered_set<uint32_t> uniqueFamilyIndicesSet(queueFamilyInfo.m_familyTypeToFamilyIndices.begin(), queueFamilyInfo.m_familyTypeToFamilyIndices.end());
            queueFamilyInfo.m_uniqueQueueFamilyIndices.assign(uniqueFamilyIndicesSet.begin(), uniqueFamilyIndicesSet.end());

            return queueFamilyInfo;
        }

        bool VkDeviceExtensionsSupported(VkPhysicalDevice vkPhysicalDevice, std::span<const char* const> extensions)
        {
            const std::vector<VkExtensionProperties> extensionsProperties = [vkPhysicalDevice]()
                {
                    // Get number of Vulkan instance extensions
                    uint32_t extensionCount = 0;
                    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, nullptr);

                    // Get list of Vulkan instance extensions supported
                    std::vector<VkExtensionProperties> extensionsProperties(extensionCount);
                    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, extensionsProperties.data());

                    return extensionsProperties;
                }();

            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(vkPhysicalDevice, &physicalDeviceProperties);
            DX_LOG(Verbose, "Vulkan Device", "Vulkan device extensions supported by '%s': %d",
                physicalDeviceProperties.deviceName, extensionsProperties.size());
            for (const auto& extensionProperties : extensionsProperties)
            {
                DX_LOG(Verbose, "Vulkan Device", "\t- %s", extensionProperties.extensionName);
            }

            return std::all_of(extensions.begin(), extensions.end(),
                [&extensionsProperties](const char* const extension)
                {
                    return std::find_if(extensionsProperties.begin(), extensionsProperties.end(),
                    [extension](const VkExtensionProperties& extensionProperties)
                        {
                            return strcmp(extensionProperties.extensionName, extension) == 0;
                        }) != extensionsProperties.end();
                });
        }

        bool CheckVkPhysicalDeviceSuitable(
            VkPhysicalDevice vkPhysicalDevice,
            VkSurfaceKHR vkSurface,
            std::span<const char* const> extensions)
        {
            // Information about the device itself (ID, name, type, vendor, etc.)
            //VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
            //vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);

            // Information about what the device can do (geometry shader, tessellation shaders, wide lines, etc.)
            //VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
            //vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkPhysicalDeviceFeatures);

            // Check device extensions support
            if (!VkDeviceExtensionsSupported(vkPhysicalDevice, extensions))
            {
                return false;
            }

            // Check Swap Chain support
            if (!SwapChain::CheckSwapChainSupported(vkPhysicalDevice, vkSurface))
            {
                return false;
            }

            // Check Queue Families support
            const QueueFamilyInfo queueFamilyInfo = EnumerateQueueFamilies(vkPhysicalDevice, vkSurface);

            return queueFamilyInfo.IsValid();
        }
    } // namespace Utils

    bool QueueFamilyInfo::IsValid() const
    {
        return std::all_of(m_familyTypeToFamilyIndices.begin(), m_familyTypeToFamilyIndices.end(),
            [](int index)
            {
                return index >= 0;
            });
    }

    Device::Device(Instance* instance)
        : m_instance(instance)
    {
        m_vkQueues.fill(nullptr);
        m_vkCommandPools.fill(nullptr);
    }

    Device::~Device()
    {
        Terminate();
    }

    bool Device::Initialize()
    {
        if (m_vkDevice)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan Device", "Initializing Vulkan Device...");

        if (!ObtainVkPhysicalDevice())
        {
            Terminate();
            return false;
        }

        if (!CreateVkDevice())
        {
            Terminate();
            return false;
        }

        if (!CreateVkCommandPools())
        {
            Terminate();
            return false;
        }

        if (!CreateVkDescriptorPool())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Device::Terminate()
    {
        DX_LOG(Info, "Vulkan Device", "Terminating Vulkan Device...");

        vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);
        m_vkDescriptorPool = nullptr;

        std::ranges::for_each(m_vkCommandPools, [this](VkCommandPool& vkCommandPool)
            {
                vkDestroyCommandPool(m_vkDevice, vkCommandPool, nullptr);
                vkCommandPool = nullptr;
            });

        vkDestroyDevice(m_vkDevice, nullptr);
        m_vkDevice = nullptr;
        m_queueFamilyInfo = QueueFamilyInfo();
        m_vkQueues.fill(nullptr);
        m_vkPhysicalDeviceProperties.reset();
        m_vkPhysicalDevice = nullptr;
    }

    void Device::WaitUntilIdle()
    {
        DX_LOG(Info, "Vulkan Device", "Waiting until device is idling...");

        // Wait until all the commands in the queues have finished executing.
        vkDeviceWaitIdle(m_vkDevice);
    }

    Instance* Device::GetInstance()
    {
        return m_instance;
    }

    VkDevice Device::GetVkDevice()
    {
        return m_vkDevice;
    }

    VkPhysicalDevice Device::GetVkPhysicalDevice()
    {
        return m_vkPhysicalDevice;
    }

    VkQueue Device::GetVkQueue(QueueFamilyType queueFamilyType)
    {
        return m_vkQueues[queueFamilyType];
    }

    VkCommandPool Device::GetVkCommandPool(QueueFamilyType queueFamilyType)
    {
        return m_vkCommandPools[queueFamilyType];
    }

    VkDescriptorPool Device::GetVkDescriptorPool()
    {
        return m_vkDescriptorPool;
    }

    const QueueFamilyInfo& Device::GetQueueFamilyInfo() const
    {
        return m_queueFamilyInfo;
    }

    const VkPhysicalDeviceProperties* Device::GetVkPhysicalDeviceProperties() const
    {
        return m_vkPhysicalDeviceProperties.get();
    }

    bool Device::ObtainVkPhysicalDevice()
    {
        // Physical devices that the Vulkan instance can access
        const std::vector<VkPhysicalDevice> physicalDevices = [this]()
            {
                uint32_t physicalDeviceCount = 0;
                vkEnumeratePhysicalDevices(m_instance->GetVkInstance(), &physicalDeviceCount, nullptr);

                std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
                vkEnumeratePhysicalDevices(m_instance->GetVkInstance(), &physicalDeviceCount, physicalDevices.data());

                return physicalDevices;
            }();

        if (physicalDevices.empty())
        {
            DX_LOG(Error, "Vulkan Device", "No physical devices found that support Vulkan instance.");
            return false;
        }
        DX_LOG(Verbose, "Vulkan Device", "Physical Devices found: %d", physicalDevices.size());
        for (const auto& physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
            DX_LOG(Verbose, "Vulkan Device", "\t- %s", physicalDeviceProperties.deviceName);
        }

        // Use first suitable Vulkan physical device
        auto physicalDeviceIt = std::find_if(physicalDevices.begin(), physicalDevices.end(),
            [this](VkPhysicalDevice physicalDevice)
            {
                return Utils::CheckVkPhysicalDeviceSuitable(physicalDevice, m_instance->GetVkSurface(), VkDeviceExtensions);
            });
        if (physicalDeviceIt == physicalDevices.end())
        {
            DX_LOG(Error, "Vulkan Device", "No suitable physical device found in Vulkan instance.");
            return false;
        }

        m_vkPhysicalDevice = *physicalDeviceIt;

        m_vkPhysicalDeviceProperties = std::make_unique<VkPhysicalDeviceProperties>();
        vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, m_vkPhysicalDeviceProperties.get());
        DX_LOG(Verbose, "Vulkan Device", "Physical Device used: %s", m_vkPhysicalDeviceProperties->deviceName);

        DX_LOG(Verbose, "Vulkan Device", "Vulkan device extensions to enable: %d", VkDeviceExtensions.size());
        for (const auto& vkDeviceExtension : VkDeviceExtensions)
        {
            DX_LOG(Verbose, "Vulkan Device", "\t- %s", vkDeviceExtension);
        }

        return true;
    }

    bool Device::CreateVkDevice()
    {
        // Queue Family information of the physical device.
        m_queueFamilyInfo = Utils::EnumerateQueueFamilies(m_vkPhysicalDevice, m_instance->GetVkSurface());
        DX_ASSERT(m_queueFamilyInfo.IsValid(), "Vulkan Device", "Queue Family Indices is not valid");

        // Populate the queues to create in the Vulkan device.
        std::vector<VkDeviceQueueCreateInfo> deviceQueuesCreateInfo(m_queueFamilyInfo.m_uniqueQueueFamilyIndices.size());
        const float queuePriority = 1.0f; // 1.0f is highest priority, 0.0f is lowest priority.
        std::transform(
            m_queueFamilyInfo.m_uniqueQueueFamilyIndices.begin(),
            m_queueFamilyInfo.m_uniqueQueueFamilyIndices.end(),
            deviceQueuesCreateInfo.begin(),
            [&queuePriority](uint32_t queueFamilyIndex)
            {
                // Create a new queue for this family index.
                VkDeviceQueueCreateInfo vkDeviceGraphicsQueueCreateInfo = {};
                vkDeviceGraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                vkDeviceGraphicsQueueCreateInfo.pNext = nullptr;
                vkDeviceGraphicsQueueCreateInfo.flags = 0;
                vkDeviceGraphicsQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                vkDeviceGraphicsQueueCreateInfo.queueCount = 1;
                vkDeviceGraphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

                return vkDeviceGraphicsQueueCreateInfo;
            });

        // Physical device features that the logical device will be using
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures = {};

        VkDeviceCreateInfo vkDeviceCreateInfo = {};
        vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        vkDeviceCreateInfo.pNext = nullptr;
        vkDeviceCreateInfo.flags = 0;
        vkDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueuesCreateInfo.size());
        vkDeviceCreateInfo.pQueueCreateInfos = deviceQueuesCreateInfo.data();
        vkDeviceCreateInfo.enabledLayerCount = 0; // Deprecated in Vulkan 1.1
        vkDeviceCreateInfo.ppEnabledLayerNames = nullptr; // Deprecated in Vulkan 1.1
        vkDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(VkDeviceExtensions.size());
        vkDeviceCreateInfo.ppEnabledExtensionNames = VkDeviceExtensions.data();
        vkDeviceCreateInfo.pEnabledFeatures = &vkPhysicalDeviceFeatures;

        if (vkCreateDevice(m_vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &m_vkDevice) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan Device", "Failed to create Vulkan device.");
            return false;
        }

        // Obtain the queues that have been created as part of the device.
        // Different family types might use the same queue family index.
        for (int familyType = 0; familyType < QueueFamilyType_Count; ++familyType)
        {
            vkGetDeviceQueue(
                m_vkDevice, 
                m_queueFamilyInfo.m_familyTypeToFamilyIndices[familyType],
                0, 
                &m_vkQueues[familyType]);
            if (!m_vkQueues[familyType])
            {
                DX_LOG(Error, "Vulkan Device", "Failed to obtain queue from Vulkan device.");
                return false;
            }
        }

        return true;
    }

    bool Device::CreateVkCommandPools()
    {
        // Create one command pool for each queue family type.
        for (int familyType = 0; familyType < QueueFamilyType_Count; ++familyType)
        {
            VkCommandPoolCreateInfo vkCommandPoolCreateInfo = {};
            vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            vkCommandPoolCreateInfo.pNext = nullptr;
            vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allows to reset command buffers
            vkCommandPoolCreateInfo.queueFamilyIndex = m_queueFamilyInfo.m_familyTypeToFamilyIndices[familyType];

            if (vkCreateCommandPool(m_vkDevice, &vkCommandPoolCreateInfo, nullptr, &m_vkCommandPools[familyType]) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Device", "Failed to create Vulkan command pool.");
                return false;
            }
        }

        return true;
    }

    bool Device::CreateVkDescriptorPool()
    {
        // Increase max counts as needed
        constexpr int maxDescriptorSetsPerFrame = 1;
        constexpr int maxUniformBufferDescriptorsPerFrame = 1;
        //constexpr int maxUniformBufferDynamicDescriptorsPerFrame = 1;

        // Max number of descriptor sets in the pool.
        // Descriptor sets contain descriptors. A descriptor can be used in different descriptor sets.
        constexpr int maxDescriptorSets = MaxFrameDraws * maxDescriptorSetsPerFrame;

        // Max number of descriptors (per type) in the pool.
        const std::vector<VkDescriptorPoolSize> vkDescriptorPoolSizes = {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = MaxFrameDraws * maxUniformBufferDescriptorsPerFrame
            },
            //{
            //    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            //    .descriptorCount = MaxFrameDraws * maxUniformBufferDynamicDescriptorsPerFrame
            //}
        };

        VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo = {};
        vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        vkDescriptorPoolCreateInfo.pNext= nullptr;
        vkDescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // To allow free descriptor sets
        vkDescriptorPoolCreateInfo.maxSets = maxDescriptorSets;
        vkDescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(vkDescriptorPoolSizes.size());
        vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSizes.data();

        if (vkCreateDescriptorPool(m_vkDevice, &vkDescriptorPoolCreateInfo, nullptr, &m_vkDescriptorPool) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan Device", "Failed to create Vulkan descriptor pool.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
