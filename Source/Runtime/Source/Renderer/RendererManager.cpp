#include <Renderer/RendererManager.h>
#include <Window/Window.h>

namespace DX
{
    RendererId RendererManager::NextRendererId = DefaultRendererId;

    Renderer* RendererManager::CreateRenderer(Window* window)
    {
        auto newRenderer = std::make_unique<Renderer>(NextRendererId, window);
        if (!newRenderer->Initialize())
        {
            return nullptr;
        }

        auto result = m_renderers.emplace(NextRendererId, std::move(newRenderer));

        NextRendererId = RendererId(NextRendererId.GetValue() + 1);

        // Result's first is the iterator to the newly inserted element (pair),
        // its second is the value of the element (unique_ptr<Window>).
        return result.first->second.get();
    }

    void RendererManager::DestroyRenderer(RendererId rendererId)
    {
        if (rendererId != DefaultRendererId)
        {
            // Removing the renderer from the map will destroy it.
            m_renderers.erase(rendererId);
        }
    }

    Renderer* RendererManager::GetRenderer(RendererId rendererId)
    {
        if (auto it = m_renderers.find(rendererId);
            it != m_renderers.end())
        {
            return it->second.get();
        }
        return nullptr;
    }
} // namespace DX