#include <Window/Window.h>
#include <Log/Log.h>

// GLFW uses Vulkan by default, so we need to indicate to not use it.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#endif

namespace DX
{
    Window::Window(WindowId windowId, std::string title, const Math::Vector2Int& size, int refreshRate, bool fullScreen, bool vSync)
        : m_windowId(windowId)
        , m_title(std::move(title))
        , m_size(size)
        , m_refreshRate(refreshRate)
        , m_fullScreen(fullScreen)
        , m_vSync(vSync)
    {
    }

    Window::~Window()
    {
        Terminate();
    }

    bool Window::Initialize()
    {
        if (m_window)
        {
            return true; // Already initialized
        }

        // Window is resizable if it's not full screen
        const bool resizeable = !m_fullScreen;

        // Do not use any client API
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, (resizeable) ? GLFW_TRUE : GLFW_FALSE);

        DX_LOG(Info, "Window", "Creating window %u with size %dx%d...", m_windowId, m_size.x, m_size.y);
        m_window = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), (m_fullScreen) ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        if (!m_window)
        {
            DX_LOG(Error, "Window", "Failed to create GLFW window.");
            return false;
        }

        // Callbacks are called for all the windows. To identify the callbacks of this window
        // we set the window's user pointer to this class.
        glfwSetWindowUserPointer(m_window, this);

        // Window resize callback
        if (resizeable)
        {
            glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
                {
                    if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window)))
                    {
                        DX_LOG(Info, "Window", "Resizing window %u to %dx%d...", self->m_windowId, width, height);

                        self->m_size.x = width;
                        self->m_size.y = height;
                        self->m_resizeEvent.Signal(self->m_size);
                    }
                });
        }

        // Accumulate mouse scroll offset
        glfwSetScrollCallback(m_window, [](GLFWwindow* window, [[maybe_unused]] double xoffset, double yoffset)
            {
                if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window)))
                {
                    self->m_scrollOffsetAccumulator += static_cast<float>(yoffset);
                }
            });

        return true;
    }

    void Window::Terminate()
    {
        if (m_window)
        {
            DX_LOG(Info, "Window", "Terminating window %u...", m_windowId);
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    bool Window::IsOpen() const
    {
        return !glfwWindowShouldClose(m_window);
    }

    HWND Window::GetWindowNativeHandler()
    {
#ifdef _WIN32
        return glfwGetWin32Window(m_window);
#else
#error "Window::GetWindowNativeHandler: Unsupported platform."
        return NULL;
#endif
    }

    void Window::PollEvents()
    {
        // Set scroll offset and reset accumulator
        m_scrollOffset = m_scrollOffsetAccumulator;
        m_scrollOffsetAccumulator = 0.0f;
    }
} // namespace DX
