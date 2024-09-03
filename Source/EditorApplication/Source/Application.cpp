#include <Application.h>

#include <Assets/AssetManager.h>
#include <Window/WindowManager.h>
#include <Renderer/RendererManager.h>
#include <Renderer/Object.h>
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

        m_renderer->SetCamera(m_camera.get());

        // Prepare render objects
        m_objects.push_back(std::make_unique<Cube>(
            Math::Transform{ {-3.0f, 0.5f, 0.0f} },
            Math::Vector3(1.0f)));
        m_objects.push_back(std::make_unique<Mesh>(
            Math::Transform{ {0.0f, 0.0f, 0.0f}, Math::Quaternion::FromEulerAngles({ 0.0f, 3.14f, 0.0f }), Math::Vector3(0.01f) },
            "Models/Jack/Jack.fbx",
            "Textures/Wall_Stone_Albedo.png",
            "Textures/Wall_Stone_Normal.png"));
        m_objects.push_back(std::make_unique<Mesh>(
            Math::Transform{ {2.0f, 1.0f, 0.0f} },
            "Models/DamagedHelmet/DamagedHelmet.gltf",
            "Models/DamagedHelmet/Default_albedo.jpg",
            "Models/DamagedHelmet/Default_normal.jpg",
            "Models/DamagedHelmet/Default_emissive.jpg"));
        m_objects.push_back(std::make_unique<Mesh>(
            Math::Transform{ {-1.5f, 0.0f, 0.0f}, Math::Quaternion::identity, Math::Vector3(0.1f) },
            "Models/Lantern/Lantern.gltf",
            "Models/Lantern/Lantern_baseColor.png",
            "Models/Lantern/Lantern_normal.png",
            "Models/Lantern/Lantern_emissive.png"));

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
