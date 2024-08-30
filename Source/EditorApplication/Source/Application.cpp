#include <Application.h>

#include <Assets/AssetManager.h>
#include <Window/WindowManager.h>
#include <Renderer/RendererManager.h>
#include <Renderer/Object.h>
#include <Camera/Camera.h>

#include <Math/Transform.h>

namespace DX
{
    namespace Quad
    {
        static const std::vector<VertexPC> VertexData1 = {
            { Math::Vector3Packed({0.4f,-0.4f, 0.0f}), Math::ColorPacked(Math::Colors::Red) },
            { Math::Vector3Packed({0.4f, 0.4f, 0.0f}), Math::ColorPacked(Math::Colors::Green) },
            { Math::Vector3Packed({-0.4f, 0.4f, 0.0f}), Math::ColorPacked(Math::Colors::Blue) },
            { Math::Vector3Packed({-0.4f,-0.4f, 0.0f}), Math::ColorPacked(Math::Colors::Yellow) },
        };

        static const std::vector<VertexPC> VertexData2 = {
            { Math::Vector3Packed({0.25f,-0.6f, 0.0f}), Math::ColorPacked(Math::Colors::Red) },
            { Math::Vector3Packed({0.25f, 0.6f, 0.0f}), Math::ColorPacked(Math::Colors::Green) },
            { Math::Vector3Packed({-0.25f, 0.6f, 0.0f}), Math::ColorPacked(Math::Colors::Blue) },
            { Math::Vector3Packed({-0.25f,-0.6f, 0.0f}), Math::ColorPacked(Math::Colors::Yellow) },
        };

        static const std::vector<Index> IndexData = { 
            0, 1, 2,
            2, 3, 0
        };
    }

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
        m_camera = std::make_unique<Camera>(Math::Vector3(0.0f, 0.0f, 2.0f), Math::Vector3(0.0f, 0.0f, 0.0f));

        m_renderer->SetCamera(m_camera.get());

        // Prepare render objects
        m_objects.push_back(std::make_unique<SimpleObject>(
            Math::Transform{ {2.0f, 0.0f, 0.0f}, Math::Quaternion::FromEulerAngles({ 0.0f, 0.0f, 3.14f/4.0f }) }, 
            Quad::VertexData1, Quad::IndexData));
        m_objects.push_back(std::make_unique<SimpleObject>(
            Math::Transform{ {1.0f, 1.0f, 0.0f} }, Quad::VertexData2, Quad::IndexData));
        m_objects.push_back(std::make_unique<Cube>(
            Math::Transform{ {0.0f, 0.0f, 0.0f} }, Math::Vector3(1.0f)));
        m_objects.push_back(std::make_unique<Cube>(
            Math::Transform{ {-2.0f, 0.0f, 0.0f} }, Math::Vector3(1.0f)));
        m_objects.push_back(std::make_unique<Cube>(
            Math::Transform{ {-3.0f, 1.0f, 0.0f} }, Math::Vector3(1.0f)));

        std::ranges::for_each(m_objects, [this](auto& object) { m_renderer->AddObject(object.get()); });

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
            for (auto& object : m_objects)
            {
                Math::Transform& transform = object->GetTransform();
                transform.m_rotation = Math::Quaternion::FromEulerAngles(Math::Vector3(0.0f, 0.5f * deltaTime, 0.0f)) * transform.m_rotation;
            }

            // ------
            // Render
            // ------
            if (m_window->IsMinimized())
            {
                // Skip rendering if window is minimized
                continue;
            }
            m_renderer->Render();
        }
    }

    void Application::Terminate()
    {
        // Necessary before destroying render objects
        if (m_renderer)
        {
            m_renderer->WaitUntilIdle();
        }

        // Clear render objects before destroying renderer manager
        m_objects.clear();

        // Destroy managers
        RendererManager::Destroy();
        WindowManager::Destroy();
        AssetManager::Destroy();
    }
}
