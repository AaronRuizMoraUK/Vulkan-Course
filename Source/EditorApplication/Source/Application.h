#pragma once

#include <Math/Vector2.h>

#include <memory>
#include <vector>

namespace DX
{
    class Window;
    class Renderer;
    class Camera;
    class Object;

    class Application
    {
    public:
        Application();
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        bool Initialize(const Math::Vector2Int& windowSize, int refreshRate = 60, bool fullScreen = false, bool vSync = false);

        void RunLoop();

        void Terminate();

    private:
        Window* m_window = nullptr;
        Renderer* m_renderer = nullptr;

        std::unique_ptr<Camera> m_camera;

        // WARNING: At the moment we're not creating/destroying dynamically
        // objects during the life of the application, they are created during
        // initialization and destroyed at termination. But if in the future
        // this vector changes dynamically, it'd have to be considered that
        // Renderer handles multiple frames at the same time and therefore
        // Object's buffers need to not be destroyed until the last frame using
        // them has finished rendering.
        std::vector<std::unique_ptr<Object>> m_objects;
    };
}
