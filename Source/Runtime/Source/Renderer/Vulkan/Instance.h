#pragma once

#include <vector>

typedef struct VkInstance_T* VkInstance;
typedef struct VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;

namespace Vulkan
{
    // Manages the Vulkan Instance and validation layers
    class Instance
    {
    public:
        Instance() = default;
        ~Instance();

        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;

        bool Initialize();
        void Terminate();

        VkInstance GetVkInstance();

    private:
        bool VkInstanceLayersSupported(const std::vector<const char*>& layers) const;
        bool VkInstanceExtensionsSupported(const std::vector<const char*>& extensions) const;

        bool CreateVkInstance();

        VkInstance m_vkInstance = nullptr;
        VkDebugUtilsMessengerEXT m_vkDebugUtilsMessenger = nullptr;
    };
} // namespace Vulkan
