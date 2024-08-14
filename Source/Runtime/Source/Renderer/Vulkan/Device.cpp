#include <Renderer/Vulkan/Device.h>

#include <Renderer/Vulkan/Instance.h>
#include <Renderer/Vulkan/SwapChain.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

#include <unordered_set>
#include <algorithm>

namespace Vulkan
{
    // Utils to extract information and perform checks on Vulkan Physical Devices
    namespace Utils
    {
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
            bool IsValid() const
            {
                return std::all_of(m_familyTypeToFamilyIndices.begin(), m_familyTypeToFamilyIndices.end(),
                    [](int index)
                    {
                        return index >= 0;
                    });
            }
        };

        VkQueueFamilyInfo EnumerateVkQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
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

            VkQueueFamilyInfo vkQueueFamilyInfo;
            for (int queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size() && !vkQueueFamilyInfo.IsValid(); ++queueFamilyIndex)
            {
                // ------------------
                // Check Graphics
                const bool queueFamilySupportsGraphics =
                    queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
                    queueFamilyProperties[queueFamilyIndex].queueCount > 0;

                // If Graphics is supported and it's the first queue family found for it, assign it.
                if (vkQueueFamilyInfo.m_familyTypeToFamilyIndices[VkQueueFamilyType_Graphics] < 0 &&
                    queueFamilySupportsGraphics)
                {
                    vkQueueFamilyInfo.m_familyTypeToFamilyIndices[VkQueueFamilyType_Graphics] = queueFamilyIndex;
                }

                // ------------------
                // Check Compute
                const bool queueFamilySupportsCompute =
                    queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT &&
                    queueFamilyProperties[queueFamilyIndex].queueCount > 0;

                // If Compute is supported and it's the first queue family found for it, assign it.
                if (vkQueueFamilyInfo.m_familyTypeToFamilyIndices[VkQueueFamilyType_Compute] < 0 &&
                    queueFamilySupportsCompute)
                {
                    vkQueueFamilyInfo.m_familyTypeToFamilyIndices[VkQueueFamilyType_Compute] = queueFamilyIndex;
                }

                // ------------------
                // Check Presentation
                VkBool32 vkQueueFamilySupportsSurface = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, queueFamilyIndex, vkSurface, &vkQueueFamilySupportsSurface);

                const bool queueFamilySupportsPresentation =
                    vkQueueFamilySupportsSurface &&
                    queueFamilyProperties[queueFamilyIndex].queueCount > 0;

                // If Presentation is supported and it's the first queue family found for it, assign it.
                if (vkQueueFamilyInfo.m_familyTypeToFamilyIndices[VkQueueFamilyType_Presentation] < 0 &&
                    queueFamilySupportsPresentation)
                {
                    vkQueueFamilyInfo.m_familyTypeToFamilyIndices[VkQueueFamilyType_Presentation] = queueFamilyIndex;
                }
            }

            return vkQueueFamilyInfo;
        }

        bool VkDeviceExtensionsSupported(VkPhysicalDevice vkPhysicalDevice, const std::vector<const char*>& extensions)
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
                [&extensionsProperties](const char* extension)
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
            const std::vector<const char*>& extensions)
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
            const VkQueueFamilyInfo vkQueueFamilyInfo = EnumerateVkQueueFamilies(vkPhysicalDevice, vkSurface);

            return vkQueueFamilyInfo.IsValid();
        }
    }

    Device::Device(Instance* instance, VkSurfaceKHR vkSurface)
        : m_instance(instance)
        , m_vkSurface(vkSurface)
    {
        m_vkQueues.fill(nullptr);
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

        if (!CreateVkDevice())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Device::Terminate()
    {
        DX_LOG(Info, "Vulkan Device", "Terminating Vulkan Device...");

        vkDestroyDevice(m_vkDevice, nullptr);
        m_vkDevice = nullptr;
        m_vkQueues.fill(nullptr);
        m_vkPhysicalDevice = nullptr;
    }

    VkDevice Device::GetVkDevice()
    {
        return m_vkDevice;
    }

    VkPhysicalDevice Device::GetVkPhysicalDevice()
    {
        return m_vkPhysicalDevice;
    }

    VkSurfaceKHR Device::GetVkSurface()
    {
        return m_vkSurface;
    }

    bool Device::CreateVkDevice()
    {
        // Vulkan device extensions that physical device must support
        const std::vector<const char*> vkDeviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        // Physical Device
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
                [this, &vkDeviceExtensions](VkPhysicalDevice physicalDevice)
                {
                    return Utils::CheckVkPhysicalDeviceSuitable(physicalDevice, m_vkSurface, vkDeviceExtensions);
                });
            if (physicalDeviceIt == physicalDevices.end())
            {
                DX_LOG(Error, "Vulkan Device", "No suitable physical device found in Vulkan instance.");
                return false;
            }

            m_vkPhysicalDevice = *physicalDeviceIt;

            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &physicalDeviceProperties);
            DX_LOG(Verbose, "Vulkan Device", "Physical Device used: %s", physicalDeviceProperties.deviceName);

            DX_LOG(Verbose, "Vulkan Device", "Vulkan device extensions to enable: %d", vkDeviceExtensions.size());
            for (const auto& vkDeviceExtension : vkDeviceExtensions)
            {
                DX_LOG(Verbose, "Vulkan Device", "\t- %s", vkDeviceExtension);
            }
        }

        // Queue Family information of the physical device.
        const Utils::VkQueueFamilyInfo vkQueueFamilyInfo = Utils::EnumerateVkQueueFamilies(m_vkPhysicalDevice, m_vkSurface);
        DX_ASSERT(vkQueueFamilyInfo.IsValid(), "Vulkan Device", "Queue Family Indices is not valid");

        const float queuePriority = 1.0f; // 1.0f is highest priority, 0.0f is lowest priority.

        // Populate the queues to create in the Vulkan device.
        std::vector<VkDeviceQueueCreateInfo> deviceQueuesCreateInfo;
        {
            deviceQueuesCreateInfo.reserve(VkQueueFamilyType_Count);

            std::unordered_set<int> familyIndicesFound; // To keep track of new family indices inside the loop
            for (int familyTypeIndex = 0; familyTypeIndex < VkQueueFamilyType_Count; ++familyTypeIndex)
            {
                const int familyIndex = vkQueueFamilyInfo.m_familyTypeToFamilyIndices[familyTypeIndex];

                // If this family index has already appeared, skip creation.
                if (familyIndicesFound.find(familyIndex) != familyIndicesFound.end())
                {
                    continue;
                }

                // New family index.
                familyIndicesFound.insert(familyIndex);

                // Create a new queue for this family index.
                VkDeviceQueueCreateInfo vkDeviceGraphicsQueueCreateInfo = {};
                vkDeviceGraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                vkDeviceGraphicsQueueCreateInfo.pNext = nullptr;
                vkDeviceGraphicsQueueCreateInfo.flags = 0;
                vkDeviceGraphicsQueueCreateInfo.queueFamilyIndex = familyIndex;
                vkDeviceGraphicsQueueCreateInfo.queueCount = 1;
                vkDeviceGraphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

                deviceQueuesCreateInfo.push_back(vkDeviceGraphicsQueueCreateInfo);
            }

            deviceQueuesCreateInfo.shrink_to_fit();
        }

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
        vkDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vkDeviceExtensions.size());
        vkDeviceCreateInfo.ppEnabledExtensionNames = vkDeviceExtensions.data();
        vkDeviceCreateInfo.pEnabledFeatures = &vkPhysicalDeviceFeatures;

        if (vkCreateDevice(m_vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &m_vkDevice) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan Device", "Failed to create Vulkan device.");
            return false;
        }

        // Obtain the queues that have been created as part of the device.
        for (int familyTypeIndex = 0; familyTypeIndex < VkQueueFamilyType_Count; ++familyTypeIndex)
        {
            vkGetDeviceQueue(
                m_vkDevice, 
                vkQueueFamilyInfo.m_familyTypeToFamilyIndices[familyTypeIndex], 
                0, 
                &m_vkQueues[familyTypeIndex]);
            if (!m_vkQueues[familyTypeIndex])
            {
                DX_LOG(Error, "Vulkan Device", "Failed to obtain queue from Vulkan device.");
                return false;
            }
        }

        return true;
    }
} // namespace Vulkan
