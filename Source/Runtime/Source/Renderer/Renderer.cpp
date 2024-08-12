#include <Renderer/Renderer.h>

#include <Log/Log.h>

#include <vulkan/vulkan.h>

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
        //if (m_device)
        //{
        //    return true; // Already initialized
        //}

        DX_LOG(Info, "Renderer", "Initializing Renderer...");

        uint32_t extensionsCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

        DX_LOG(Info, "Vulkan", "Number of extensions: %i", extensionsCount);

        return true;
    }

    void Renderer::Terminate()
    {
        DX_LOG(Info, "Renderer", "Terminating Renderer...");

        m_window->UnregisterWindowResizeEvent(m_windowResizeHandler);
    }

    Window* Renderer::GetWindow()
    {
        return m_window;
    }
} // namespace DX
