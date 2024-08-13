#pragma once

#include <Window/Window.h>
#include <GenericId/GenericId.h>

typedef struct VkInstance_T* VkInstance;
typedef struct VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;

namespace DX
{
    using RendererId = GenericId<struct RendererIdTag>;

    // Manages the render device, swap chain, frame buffer and scene.
    class Renderer
    {
    public:
        Renderer(RendererId rendererId, Window* window);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        bool Initialize();
        void Terminate();

        RendererId GetId() const { return m_rendererId; }

        Window* GetWindow();

        //void Render();
        //void Present();

    private:
        RendererId m_rendererId;
        Window* m_window = nullptr;
        WindowResizeEvent::Handler m_windowResizeHandler;

    private:
        // Location of Queue Families in a Vulkan physical device
        struct VkQueueFamilyIndices
        {
            int m_graphicsFamily = -1;

            // Check if all families have been found
            bool IsValid() const
            {
                return m_graphicsFamily >= 0;
            }
        };

        // All Vulkan information associated with a Vulkan logical device
        struct VkLogicalDevice
        {
            VkPhysicalDevice m_vkPhysicalDevice = nullptr;
            VkQueueFamilyIndices m_vkQueueFamilyIndices;
            VkDevice m_vkDevice = nullptr;
            VkQueue m_vkGraphicsQueue = nullptr;

            void Terminate();
        };

        bool VkInstanceLayersSupported(const std::vector<const char*>& layers) const;
        bool VkInstanceExtensionsSupported(const std::vector<const char*>& extensions) const;
        bool CheckVkPhysicalDeviceSuitable(VkPhysicalDevice vkPhysicalDevice) const;
        VkQueueFamilyIndices EnumerateVkQueueFamilies(VkPhysicalDevice vkPhysicalDevice) const;

        bool CreateVkInstance();
        bool CreateVkLogicalDevice();

        VkInstance m_vkInstance = nullptr;
        VkDebugUtilsMessengerEXT m_vkDebugUtilsMessenger = nullptr;
        VkLogicalDevice m_vkLogicalDevice;
    };
} // namespace DX
