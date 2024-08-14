#include <Renderer/Renderer.h>

#include <Renderer/Vulkan/Instance.h>
#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/SwapChain.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

// Necessary to ask GLFW to create a Vulkan surface for the window
#define GLFW_INCLUDE_VULKAN // This will cause glfw3.h to include vulkan.h already
#include <GLFW/glfw3.h>

namespace DX
{
    Renderer::Renderer(RendererId rendererId, Window* window)
        : m_rendererId(rendererId)
        , m_window(window)
    {
    }

    Renderer::~Renderer()
    {
        Terminate();
    }

    bool Renderer::Initialize()
    {
        if (m_device)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Renderer", "Initializing Renderer...");

        if (!CreateInstance())
        {
            Terminate();
            return false;
        }

        if (!CreateVkSurface())
        {
            Terminate();
            return false;
        }

        if (!CreateDevice())
        {
            Terminate();
            return false;
        }

        if (!CreateSwapChain())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Renderer::Terminate()
    {
        DX_LOG(Info, "Renderer", "Terminating Renderer...");

        m_window->UnregisterWindowResizeEvent(m_windowResizeHandler);

        m_swapChain.reset();
        m_device.reset();
        vkDestroySurfaceKHR((m_instance) ? m_instance->GetVkInstance() : nullptr, m_vkSurface, nullptr);
        m_vkSurface = nullptr;
        m_instance.reset();
    }

    Window* Renderer::GetWindow()
    {
        return m_window;
    }

    bool Renderer::CreateInstance()
    {
        m_instance = std::make_unique<Vulkan::Instance>();

        if (!m_instance->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create instance.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateVkSurface()
    {
        // It uses GLFW library to create the Vulkan surface for the window it handles.
        // The surface creates must match the operative system, for example on Windows
        // it will use vkCreateWin32SurfaceKHR. GLFW handles this automatically and creates
        // the appropriate Vulkan surface.
        if (glfwCreateWindowSurface(m_instance->GetVkInstance(), m_window->GetWindowHandler(), nullptr, &m_vkSurface) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to create Vulkan surface for window.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateDevice()
    {
        m_device = std::make_unique<Vulkan::Device>(m_instance.get(), m_vkSurface);

        if (!m_device->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create device.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateSwapChain()
    {
        // Get frame buffer size in pixels from GLWF
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window->GetWindowHandler(), &width, &height);

        m_swapChain = std::make_unique<Vulkan::SwapChain>(m_device.get(), Math::Vector2Int(width, height));

        if (!m_swapChain->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create swap chain.");
            return false;
        }

        return true;
    }
} // namespace DX
