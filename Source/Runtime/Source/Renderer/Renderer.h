#pragma once

#include <Window/Window.h>
#include <GenericId/GenericId.h>

#include <memory>

namespace Vulkan
{
    class Instance;
    class Device;
}

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
        bool CreateInstance();
        bool CreateDevice();

        std::unique_ptr<Vulkan::Instance> m_instance;
        std::unique_ptr<Vulkan::Device> m_device;
    };
} // namespace DX
