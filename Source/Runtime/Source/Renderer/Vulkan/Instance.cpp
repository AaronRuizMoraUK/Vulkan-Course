#include <Renderer/Vulkan/Instance.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

// Necessary to ask GLFW:
// - What Vulkan instance extensions it needs
// - To create a Vulkan surface for the window
#define GLFW_INCLUDE_VULKAN // This will cause glfw3.h to include vulkan.h already
#include <GLFW/glfw3.h>

#include <vector>
#include <algorithm>

namespace Vulkan
{
    // Helpers for Vulkan Validation and log validation messages
    namespace Validation
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
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
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
    } // namespace Validation

    // Utils to extract information and perform checks on Vulkan Instances
    namespace Utils
    {
        bool VkInstanceLayersSupported(const std::vector<const char*>& layers)
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

            DX_LOG(Verbose, "Vulkan Instance", "Vulkan instance layers supported: %d", layersProperties.size());
            for (const auto& layerProperties : layersProperties)
            {
                DX_LOG(Verbose, "Vulkan Instance", "\t- %s", layerProperties.layerName);
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

        bool VkInstanceExtensionsSupported(const std::vector<const char*>& extensions)
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

            DX_LOG(Verbose, "Vulkan Instance", "Vulkan instance extensions supported: %d", extensionsProperties.size());
            for (const auto& extensionProperties : extensionsProperties)
            {
                DX_LOG(Verbose, "Vulkan Instance", "\t- %s", extensionProperties.extensionName);
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
    } // namespace Utils

    Instance::Instance(GLFWwindow* windowHandler)
        : m_windowHandler(windowHandler)
    {
    }

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

        if (!CreateVkSurface())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Instance::Terminate()
    {
        DX_LOG(Info, "Vulkan Instance", "Terminating Vulkan Instance...");

        vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
        m_vkSurface = nullptr;

        Validation::DestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugUtilsMessenger, nullptr);
        m_vkDebugUtilsMessenger = nullptr;

        vkDestroyInstance(m_vkInstance, nullptr);
        m_vkInstance = nullptr;
    }

    GLFWwindow* Instance::GetWindowHandler()
    {
        return m_windowHandler;
    }

    VkInstance Instance::GetVkInstance()
    {
        return m_vkInstance;
    }

    VkSurfaceKHR Instance::GetVkSurface()
    {
        return m_vkSurface;
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
            if (Validation::DebugEnabled)
            {
                // Enable Vulkan validation layers
                vkInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
            }
        }
        if (!Utils::VkInstanceLayersSupported(vkInstanceLayers))
        {
            DX_LOG(Error, "Vulkan Instance", "Vulkan instance layers used are not supported.");
            return false;
        }
        DX_LOG(Verbose, "Vulkan Instance", "Vulkan instance layers to enable: %d", vkInstanceLayers.size());
        for (const auto& vkInstanceLayer : vkInstanceLayers)
        {
            DX_LOG(Verbose, "Vulkan Instance", "\t- %s", vkInstanceLayer);
        }

        // Vulkan instance extensions
        std::vector<const char*> vkInstanceExtensions;
        {
            // Extensions required by GLFW.
            // Names returned are valid until GLFW is terminated.
            uint32_t glfwVkInstanceExtensionCount = 0;
            const char** glfwVkInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwVkInstanceExtensionCount);
            vkInstanceExtensions.assign(glfwVkInstanceExtensions, glfwVkInstanceExtensions + glfwVkInstanceExtensionCount);

            if (Validation::DebugEnabled)
            {
                // Enable debug extension to be able to specify a callback to log validation errors
                vkInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
        }
        if (!Utils::VkInstanceExtensionsSupported(vkInstanceExtensions))
        {
            DX_LOG(Error, "Vulkan Instance", "Vulkan instance extensions used are not supported.");
            return false;
        }
        DX_LOG(Verbose, "Vulkan Instance", "Vulkan instance extensions to enable: %d", vkInstanceExtensions.size());
        for (const auto& vkInstanceExtension : vkInstanceExtensions)
        {
            DX_LOG(Verbose, "Vulkan Instance", "\t- %s", vkInstanceExtension);
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {}; // Here to ensure it is not destroyed before vkCreateInstance call

        VkInstanceCreateInfo vkInstanceCreateInfo = {};
        vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        if (Validation::DebugEnabled)
        {
            // Include debug create info here so it also logs validation messages
            // for vkCreateInstance and vkDestroyInstance functions.
            debugCreateInfo = Validation::DebugUtilsMessengerCreateInfo();
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
            DX_LOG(Error, "Vulkan Instance", "Failed to create Vulkan instance.");
            return false;
        }

        // Create debug utils messenger to set callback to print Vulkan validation messages
        if (Validation::DebugEnabled)
        {
            if (Validation::CreateDebugUtilsMessengerEXT(m_vkInstance, nullptr, &m_vkDebugUtilsMessenger) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Instance", "Failed to create Vulkan debug utils messenger.");
                return false;
            }
        }

        return true;
    }

    bool Instance::CreateVkSurface()
    {
        // It uses GLFW library to create the Vulkan surface for the window it handles.
        // The surface creates must match the operative system, for example on Windows
        // it will use vkCreateWin32SurfaceKHR. GLFW handles this automatically and creates
        // the appropriate Vulkan surface.
        if (glfwCreateWindowSurface(m_vkInstance, m_windowHandler, nullptr, &m_vkSurface) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan Instance", "Failed to create Vulkan surface for the window.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
