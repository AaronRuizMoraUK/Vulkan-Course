#pragma once

typedef struct GLFWwindow GLFWwindow;
typedef struct VkInstance_T* VkInstance;
typedef struct VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;

namespace Vulkan
{
    // Manages the Vulkan Instance and validation layers
    class Instance
    {
    public:
        Instance(GLFWwindow* windowHandler);
        ~Instance();

        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;

        bool Initialize();
        void Terminate();

        GLFWwindow* GetWindowHandler();
        VkInstance GetVkInstance();
        VkSurfaceKHR GetVkSurface();

    private:
        GLFWwindow* m_windowHandler = nullptr;

    private:
        bool CreateVkInstance();
        bool CreateVkSurface();

        VkInstance m_vkInstance = nullptr;
        VkDebugUtilsMessengerEXT m_vkDebugUtilsMessenger = nullptr;

        VkSurfaceKHR m_vkSurface = nullptr;
    };
} // namespace Vulkan
