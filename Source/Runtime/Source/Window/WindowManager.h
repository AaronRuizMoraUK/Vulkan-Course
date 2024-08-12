#pragma once

#include <Singleton/Singleton.h>
#include <Window/Window.h>

#include <memory>
#include <unordered_map>

namespace DX
{
    class WindowManager : public Singleton<WindowManager>
    {
        friend class Singleton<WindowManager>;
        WindowManager();

    public:
        // First window created will become the default one.
        // Default window cannot be destroyed with DestroyWindow.
        static inline const WindowId DefaultWindowId{ 1 };

        ~WindowManager();

        Window* CreateWindowWithTitle(std::string title, const Math::Vector2Int& size, int refreshRate = 60, bool fullScreen = false, bool vSync = false);
        void DestroyWindow(WindowId windowId);

        Window* GetWindow(WindowId windowId = DefaultWindowId);

        void PollEvents();

    private:
        static WindowId NextWindowId;

        using Windows = std::unordered_map<WindowId, std::unique_ptr<Window>>;
        Windows m_windows;
    };
} // namespace DX