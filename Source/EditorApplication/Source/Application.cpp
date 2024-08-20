#include <Application.h>

#include <Assets/AssetManager.h>
#include <Window/WindowManager.h>
#include <Renderer/RendererManager.h>
#include <Camera/Camera.h>

#include <Math/Transform.h>

namespace DX
{
    Application::Application() = default;

    Application::~Application() = default;

    bool Application::Initialize(const Math::Vector2Int& windowSize, int refreshRate, bool fullScreen, bool vSync)
    {
        // Asset Manager initialization
        AssetManager::Get();

        // Window Manager initialization
        m_window = WindowManager::Get().CreateWindowWithTitle("Vulkan Course", windowSize, refreshRate, fullScreen, vSync);
        if (!m_window)
        {
            Terminate();
            return false;
        }

        // Renderer Manager initialization
        m_renderer = RendererManager::Get().CreateRenderer(m_window);
        if (!m_renderer)
        {
            Terminate();
            return false;
        }

        // Camera
        m_camera = std::make_unique<Camera>(Math::Vector3(0.0f, 2.0f, -2.0f), Math::Vector3(0.0f, 1.0f, 0.0f));

        return true;
    }

    void Application::RunLoop()
    {
        auto t0 = std::chrono::system_clock::now();

        while (m_window->IsOpen())
        {
            WindowManager::Get().PollEvents();

            // Calculate delta time
            const auto t1 = std::chrono::system_clock::now();
            const float deltaTime = std::chrono::duration<float>(t1 - t0).count();
            t0 = t1;
            //DX_LOG(Verbose, "Main", "Delta time: %f FPS: %0.1f", deltaTime, 1.0f / deltaTime);

            // ------
            // Update
            // ------
            m_camera->Update(deltaTime);

            // ------
            // Render
            // ------
            m_renderer->Render();
        }
    }

    void Application::Terminate()
    {
        // Destroy managers
        RendererManager::Destroy();
        WindowManager::Destroy();
        AssetManager::Destroy();
    }
}
