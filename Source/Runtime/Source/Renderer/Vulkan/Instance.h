#pragma once

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
        bool CreateVkInstance();

        VkInstance m_vkInstance = nullptr;
        VkDebugUtilsMessengerEXT m_vkDebugUtilsMessenger = nullptr;
    };
} // namespace Vulkan
