#pragma once

#include <Singleton/Singleton.h>
#include <Renderer/Renderer.h>

#include <memory>
#include <unordered_map>

namespace DX
{
    class Window;

    class RendererManager : public Singleton<RendererManager>
    {
        friend class Singleton<RendererManager>;
        RendererManager() = default;

    public:
        // First renderer created will become the default one.
        // Default renderer cannot be destroyed with DestroyRenderer.
        static inline const RendererId DefaultRendererId{ 1 };

        ~RendererManager() = default;

        Renderer* CreateRenderer(Window* window);
        void DestroyRenderer(RendererId rendererId);

        Renderer* GetRenderer(RendererId rendererId = DefaultRendererId);

    private:
        static RendererId NextRendererId;

        using Renderers = std::unordered_map<RendererId, std::unique_ptr<Renderer>>;
        Renderers m_renderers;
    };
} // namespace DX
