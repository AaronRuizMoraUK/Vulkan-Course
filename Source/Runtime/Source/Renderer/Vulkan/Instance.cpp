#include <Renderer/Vulkan/Instance.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

// Necessary to ask GLFW what Vulkan instance extensions it needs
#define GLFW_INCLUDE_VULKAN // This will cause glfw3.h to include vulkan.h already
#include <GLFW/glfw3.h>

#include <algorithm>

// Helper for Vulkan Validation and log validation messages
namespace VkValidation
{
#ifdef _DEBUG
    static const bool DebugEnabled = true;
#else
    static const bool DebugEnabled = false;
#endif

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        [[maybe_unused]] void* userData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            DX_LOG(Verbose, "Vulkan Debug", callbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            DX_LOG(Info, "Vulkan Debug", callbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            DX_LOG(Warning, "Vulkan Debug", callbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            DX_LOG(Error, "Vulkan Debug", callbackData->pMessage);
        }

        return VK_FALSE;
    }

    static VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo()
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.pNext = nullptr;
        debugCreateInfo.flags = 0;
        debugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;;
        debugCreateInfo.pfnUserCallback = DebugCallback;
        debugCreateInfo.pUserData = nullptr;

        return debugCreateInfo;
    }

    static VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            const VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = DebugUtilsMessengerCreateInfo();

            return func(instance, &debugCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
}

namespace Vulkan
{
    Instance::~Instance()
    {
        Terminate();
    }

    bool Instance::Initialize()
    {
        if (m_vkInstance)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan Instance", "Initializing Vulkan Instance...");

        if (!CreateVkInstance())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Instance::Terminate()
    {
        DX_LOG(Info, "Vulkan Instance", "Terminating Vulkan Instance...");

        VkValidation::DestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugUtilsMessenger, nullptr);
        m_vkDebugUtilsMessenger = nullptr;

        vkDestroyInstance(m_vkInstance, nullptr);
        m_vkInstance = nullptr;
    }

    VkInstance Instance::GetVkInstance()
    {
        return m_vkInstance;
    }

    bool Instance::VkInstanceLayersSupported(const std::vector<const char*>& layers) const
    {
        const std::vector<VkLayerProperties> layersProperties = []()
            {
                // Get number of Vulkan instance layers
                uint32_t layerCount = 0;
                vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

                // Get list of Vulkan instance layers supported
                std::vector<VkLayerProperties> layersProperties(layerCount);
                vkEnumerateInstanceLayerProperties(&layerCount, layersProperties.data());

                return layersProperties;
            }();

        DX_LOG(Verbose, "Renderer", "Vulkan instance layers supported: %d", layersProperties.size());
        for (const auto& layerProperties : layersProperties)
        {
            DX_LOG(Verbose, "Renderer", "\t- %s", layerProperties.layerName);
        }

        return std::all_of(layers.begin(), layers.end(),
            [&layersProperties](const char* extension)
            {
                return std::find_if(layersProperties.begin(), layersProperties.end(),
                    [extension](const VkLayerProperties& layerProperties)
                    {
                        return strcmp(layerProperties.layerName, extension) == 0;
                    }) != layersProperties.end();
            });
    }

    bool Instance::VkInstanceExtensionsSupported(const std::vector<const char*>& extensions) const
    {
        const std::vector<VkExtensionProperties> extensionsProperties = []()
            {
                // Get number of Vulkan instance extensions
                uint32_t extensionCount = 0;
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

                // Get list of Vulkan instance extensions supported
                std::vector<VkExtensionProperties> extensionsProperties(extensionCount);
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsProperties.data());

                return extensionsProperties;
            }();

        DX_LOG(Verbose, "Renderer", "Vulkan instance extensions supported: %d", extensionsProperties.size());
        for (const auto& extensionProperties : extensionsProperties)
        {
            DX_LOG(Verbose, "Renderer", "\t- %s", extensionProperties.extensionName);
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

    bool Instance::CreateVkInstance()
    {
        VkApplicationInfo vkAppInfo = {};
        vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        vkAppInfo.pNext = nullptr;
        vkAppInfo.pApplicationName = "Vulkan Course";
        vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        vkAppInfo.pEngineName = "DX";
        vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        vkAppInfo.apiVersion = VK_API_VERSION_1_3; // Vulkan API version to be using.

        // Vulkan instance layers
        std::vector<const char*> vkInstanceLayers;
        {
            if (VkValidation::DebugEnabled)
            {
                // Enable Vulkan validation layers
                vkInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
            }
        }
        if (!VkInstanceLayersSupported(vkInstanceLayers))
        {
            DX_LOG(Error, "Renderer", "Vulkan instance layers used are not supported.");
            return false;
        }
        DX_LOG(Verbose, "Renderer", "Vulkan instance layers enabled: %d", vkInstanceLayers.size());
        for (const auto& vkInstanceLayer : vkInstanceLayers)
        {
            DX_LOG(Verbose, "Renderer", "\t- %s", vkInstanceLayer);
        }

        // Vulkan instance extensions
        std::vector<const char*> vkInstanceExtensions;
        {
            // Extensions required by GLFW.
            // Names returned are valid until GLFW is terminated.
            uint32_t glfwVkInstanceExtensionCount = 0;
            const char** glfwVkInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwVkInstanceExtensionCount);
            vkInstanceExtensions.assign(glfwVkInstanceExtensions, glfwVkInstanceExtensions + glfwVkInstanceExtensionCount);

            if (VkValidation::DebugEnabled)
            {
                // Enable debug extension to be able to specify a callback to log validation errors
                vkInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
        }
        if (!VkInstanceExtensionsSupported(vkInstanceExtensions))
        {
            DX_LOG(Error, "Renderer", "Vulkan instance extensions used are not supported.");
            return false;
        }
        DX_LOG(Verbose, "Renderer", "Vulkan instance extensions enabled: %d", vkInstanceExtensions.size());
        for (const auto& vkInstanceExtension : vkInstanceExtensions)
        {
            DX_LOG(Verbose, "Renderer", "\t- %s", vkInstanceExtension);
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo; // Here to ensure it is not destroyed before vkCreateInstance call

        VkInstanceCreateInfo vkInstanceCreateInfo = {};
        vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        if (VkValidation::DebugEnabled)
        {
            // Include debug create info here so it also logs validation messages
            // for vkCreateInstance and vkDestroyInstance functions.
            debugCreateInfo = VkValidation::DebugUtilsMessengerCreateInfo();
            vkInstanceCreateInfo.pNext = &debugCreateInfo;
        }
        else
        {
            vkInstanceCreateInfo.pNext = nullptr;
        }
        vkInstanceCreateInfo.flags = 0;
        vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;
        vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(vkInstanceLayers.size());
        vkInstanceCreateInfo.ppEnabledLayerNames = vkInstanceLayers.data();
        vkInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vkInstanceExtensions.size());
        vkInstanceCreateInfo.ppEnabledExtensionNames = vkInstanceExtensions.data();

        if (vkCreateInstance(&vkInstanceCreateInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to create Vulkan instance.");
            return false;
        }

        // Create debug utils messenger to set callback to print Vulkan validation messages
        if (VkValidation::DebugEnabled)
        {
            if (VkValidation::CreateDebugUtilsMessengerEXT(m_vkInstance, nullptr, &m_vkDebugUtilsMessenger) != VK_SUCCESS)
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan debug utils messenger.");
                return false;
            }
        }

        return true;
    }
} // namespace Vulkan
