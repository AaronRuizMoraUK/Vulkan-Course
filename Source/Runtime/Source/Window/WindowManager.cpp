#include <Window/WindowManager.h>
#include <Log/Log.h>

// GLFW uses Vulkan by default, so we need to indicate to not use it.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <ranges>

namespace DX
{
    WindowId WindowManager::NextWindowId = DefaultWindowId;

    WindowManager::WindowManager()
    {
        DX_LOG(Info, "Window Manager", "Initializing GLFW...");
        if (!glfwInit())
        {
            DX_LOG(Fatal, "Window Manager", "Failed to initialize GLFW.");
        }
    }

    WindowManager::~WindowManager()
    {
        // Clearing the map will destroy all windows.
        m_windows.clear();

        DX_LOG(Info, "Window Manager", "Terminating GLFW...");
        glfwTerminate();
    }

    Window* WindowManager::CreateWindowWithTitle(std::string title, const Math::Vector2Int& size, int refreshRate, bool fullScreen, bool vSync)
    {
        auto newWindow = std::make_unique<Window>(NextWindowId, std::move(title), size, refreshRate, fullScreen, vSync);
        if (!newWindow->Initialize())
        {
            return nullptr;
        }

        auto result = m_windows.emplace(NextWindowId, std::move(newWindow));

        NextWindowId = WindowId(NextWindowId.GetValue() + 1);

        // Result's first is the iterator to the newly inserted element (pair),
        // its second is the value of the element (unique_ptr<Window>).
        return result.first->second.get();
    }

    void WindowManager::DestroyWindow(WindowId windowId)
    {
        if (windowId != DefaultWindowId)
        {
            // Removing the window from the map will destroy it.
            m_windows.erase(windowId);
        }
    }

    Window* WindowManager::GetWindow(WindowId windowId)
    {
        if (auto it = m_windows.find(windowId);
            it != m_windows.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    void WindowManager::PollEvents()
    {
        glfwPollEvents();

        for (auto& window : m_windows | std::views::values)
        {
            window->PollEvents();
        }
    }
} // namespace DX