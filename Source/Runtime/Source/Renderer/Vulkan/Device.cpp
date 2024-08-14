#include <Renderer/Vulkan/Device.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    Device::Device(Instance* instance)
        : m_instance(instance)
    {
    }

    Device::~Device()
    {
        Terminate();
    }

    bool Device::Initialize()
    {
        if (m_vkLogicalDevice.m_vkDevice)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan Device", "Initializing Vulkan Device...");

        if (!CreateVkLogicalDevice())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Device::Terminate()
    {
        DX_LOG(Info, "Vulkan Device", "Terminating Vulkan Device...");

        m_vkLogicalDevice.Terminate();
    }

    VkDevice Device::GetVkDevice()
    {
        return m_vkLogicalDevice.m_vkDevice;
    }

    void Device::VkLogicalDevice::Terminate()
    {
        vkDestroyDevice(m_vkDevice, nullptr);
        m_vkDevice = nullptr;
        m_vkGraphicsQueue = nullptr;
        m_vkPhysicalDevice = nullptr;
        m_vkQueueFamilyIndices = VkQueueFamilyIndices();
    }

    bool Device::CheckVkPhysicalDeviceSuitable(VkPhysicalDevice vkPhysicalDevice) const
    {
        // Information about the device itself (ID, name, type, vendor, etc.)
        //VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        //vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);

        // Information about what the device can do (geometry shader, tessellation shaders, wide lines, etc.)
        //VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
        //vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkPhysicalDeviceFeatures);

        const VkQueueFamilyIndices vkQueueFamilyIndices = EnumerateVkQueueFamilies(vkPhysicalDevice);

        return vkQueueFamilyIndices.IsValid();
    }

    Device::VkQueueFamilyIndices Device::EnumerateVkQueueFamilies(VkPhysicalDevice vkPhysicalDevice) const
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

        VkQueueFamilyIndices vkQueueFamilyIndices;
        for (size_t i = 0; i < queueFamilyProperties.size() && !vkQueueFamilyIndices.IsValid(); ++i)
        {
            if (vkQueueFamilyIndices.m_graphicsFamily < 0 &&
                queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
                queueFamilyProperties[i].queueCount > 0)
            {
                vkQueueFamilyIndices.m_graphicsFamily = static_cast<int>(i);
            }
        }

        return vkQueueFamilyIndices;
    }

    bool Device::CreateVkLogicalDevice()
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
            DX_LOG(Error, "Renderer", "No physical devices found that support Vulkan instance.");
            return false;
        }
        DX_LOG(Verbose, "Renderer", "Physical Devices found: %d", physicalDevices.size());
        for (const auto& physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
            DX_LOG(Verbose, "Renderer", "\t- %s", physicalDeviceProperties.deviceName);
        }

        // Use first suitable Vulkan physical device
        auto physicalDeviceIt = std::find_if(physicalDevices.begin(), physicalDevices.end(),
            [this](VkPhysicalDevice physicalDevice)
            {
                return CheckVkPhysicalDeviceSuitable(physicalDevice);
            });
        if (physicalDeviceIt != physicalDevices.end())
        {
            m_vkLogicalDevice.m_vkPhysicalDevice = *physicalDeviceIt;
            m_vkLogicalDevice.m_vkQueueFamilyIndices = EnumerateVkQueueFamilies(m_vkLogicalDevice.m_vkPhysicalDevice);
            DX_ASSERT(m_vkLogicalDevice.m_vkQueueFamilyIndices.IsValid(), "Renderer", "Queue Family Indices is not valid");

            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(m_vkLogicalDevice.m_vkPhysicalDevice, &physicalDeviceProperties);
            DX_LOG(Verbose, "Renderer", "Physical Device used: %s", physicalDeviceProperties.deviceName);
        }
        else
        {
            DX_LOG(Error, "Renderer", "No suitable physical device found in Vulkan instance.");
            return false;
        }

        // Populate the queues to create in the Vulkan device.
        // 1.0f is highest priority, 0.0f is lowest priority.
        const float graphicsQueuePriority = 1.0f;

        VkDeviceQueueCreateInfo vkDeviceGraphicsQueueCreateInfo = {};
        vkDeviceGraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        vkDeviceGraphicsQueueCreateInfo.pNext = nullptr;
        vkDeviceGraphicsQueueCreateInfo.flags = 0;
        vkDeviceGraphicsQueueCreateInfo.queueFamilyIndex = m_vkLogicalDevice.m_vkQueueFamilyIndices.m_graphicsFamily;
        vkDeviceGraphicsQueueCreateInfo.queueCount = 1;
        vkDeviceGraphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;

        const std::vector<VkDeviceQueueCreateInfo> deviceQueuesCreateInfo = {
            vkDeviceGraphicsQueueCreateInfo
        };

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
        vkDeviceCreateInfo.enabledExtensionCount = 0;
        vkDeviceCreateInfo.ppEnabledExtensionNames = nullptr;
        vkDeviceCreateInfo.pEnabledFeatures = &vkPhysicalDeviceFeatures;

        if (vkCreateDevice(m_vkLogicalDevice.m_vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &m_vkLogicalDevice.m_vkDevice) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to create Vulkan device.");
            return false;
        }

        // Obtain the queues that have been created as part of the device.
        vkGetDeviceQueue(m_vkLogicalDevice.m_vkDevice,
            m_vkLogicalDevice.m_vkQueueFamilyIndices.m_graphicsFamily,
            0,
            &m_vkLogicalDevice.m_vkGraphicsQueue);
        if (!m_vkLogicalDevice.m_vkGraphicsQueue)
        {
            DX_LOG(Error, "Renderer", "Failed to obtain graphics queue from Vulkan device.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
