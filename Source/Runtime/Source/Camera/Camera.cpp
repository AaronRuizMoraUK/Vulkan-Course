#include <Camera/Camera.h>

#include <Window/WindowManager.h>
#include <Debug/Debug.h>

#include <Math/Vector2.h>
#include <Math/Matrix3x3.h>
#include <mathfu/constants.h>

// GLFW uses Vulkan by default, so we need to indicate to not use it.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace DX
{
    Camera::Camera() = default;

    Camera::Camera(const Math::Vector3& position, const Math::Vector3& lookAtPosition)
    {
        m_transform.m_position = position;
        if (const auto basisZ = (lookAtPosition - position);
            basisZ.Length() > 1e-5f)
        {
            m_transform.m_rotation = Math::CreateQuatFromBasisZ(basisZ.Normalized());
        }
    }

    Camera::Camera(const Math::Transform& transform)
        : m_transform(transform)
    {
    }

    Camera::~Camera() = default;

    void Camera::Update(float deltaTime)
    {
        Window* window = WindowManager::Get().GetWindow();
        DX_ASSERT(window, "Camera", "Default window not found");

        auto* windowHandler = window->GetWindowHandler();

        if (glfwGetMouseButton(windowHandler, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            // Hide mouse cursor
            glfwSetInputMode(windowHandler, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }
        else if (glfwGetMouseButton(windowHandler, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
        {
            // Show mouse cursor
            glfwSetInputMode(windowHandler, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            m_firstUpdate = true;
            return;
        }

        // Speed
        {
            m_moveSpeed = std::clamp(m_moveSpeed + 0.2f * window->GetScrollOffset(), 0.1f, 1000.0f);
        }

        // Movement
        {
            Math::Vector3 deltaMovement(0.0f);

            // Forward/Backward
            if (glfwGetKey(windowHandler, GLFW_KEY_W) == GLFW_PRESS)
            {
                deltaMovement += m_transform.GetBasisZ();
            }
            if (glfwGetKey(windowHandler, GLFW_KEY_S) == GLFW_PRESS)
            {
                deltaMovement -= m_transform.GetBasisZ();
            }
            // Left/Right (LeftHand)
            if (glfwGetKey(windowHandler, GLFW_KEY_A) == GLFW_PRESS)
            {
                deltaMovement -= m_transform.GetBasisX();
            }
            if (glfwGetKey(windowHandler, GLFW_KEY_D) == GLFW_PRESS)
            {
                deltaMovement += m_transform.GetBasisX();
            }

            // Up/Down
            if (glfwGetKey(windowHandler, GLFW_KEY_E) == GLFW_PRESS)
            {
                deltaMovement += mathfu::kAxisY3f;
            }
            if (glfwGetKey(windowHandler, GLFW_KEY_Q) == GLFW_PRESS)
            {
                deltaMovement -= mathfu::kAxisY3f;
            }

            m_transform.m_position += deltaMovement * m_moveSpeed * deltaTime;
        }

        // Rotation
        {
            // Division by 2 done with integers to lose the decimals, necessary to avoid issues
            // calculating delta when width is not multiple of 2 as glfwGetCursorPos doesn't return decimals.
            const Math::Vector2 windowHalfSize(
                static_cast<float>(window->GetSize().x / 2), 
                static_cast<float>(window->GetSize().y / 2));

            if (m_firstUpdate)
            {
                glfwSetCursorPos(windowHandler, windowHalfSize.x, windowHalfSize.y);
                m_firstUpdate = false;
            }

            double mouseX = 0.0f;
            double mouseY = 0.0f;
            glfwGetCursorPos(windowHandler, &mouseX, &mouseY);

            const Math::Vector2 mousePosition(
                static_cast<float>(mouseX),
                static_cast<float>(mouseY));

            Math::Vector2 delta = m_rotationSensitivity * (mousePosition - windowHalfSize) / (2.0f * windowHalfSize);

            // Clamp pitch to avoid camera looking straight up or down.
            // Negative pitch delta makes camera look higher.
            const float angle = acosf(Math::Vector3::DotProduct(m_transform.GetBasisZ(), mathfu::kAxisY3f)) * mathfu::kRadiansToDegrees;
            const float minAngle = 10.0f;
            if ((angle <= minAngle && delta.y < 0.0f) ||          // Avoid looking higher than 10 degrees to up axis
                (angle >= (180.0f - minAngle) && delta.y > 0.0f)) // Avoid looking lower  than 10 degrees to -up axis
            {
                delta.y = 0.0f;
            }

            m_transform.m_rotation =
                Math::Quaternion::FromAngleAxis(delta.x, mathfu::kAxisY3f) * // Apply yaw in world space to orbit around up axis (LeftHand)
                m_transform.m_rotation * 
                Math::Quaternion::FromAngleAxis(delta.y, mathfu::kAxisX3f); // Apply pitch in local space

            // Reset cursor position to the center of the window
            glfwSetCursorPos(windowHandler, windowHalfSize.x, windowHalfSize.y);
        }
    }

    Math::Matrix4x4 Camera::GetViewMatrix() const
    {
        return Math::Matrix4x4::LookAt(
            m_transform.m_position + m_transform.GetBasisZ(),
            m_transform.m_position,
            m_transform.GetBasisY(),
            Math::CoordinateSystem::Default);
    }

    Math::Matrix4x4 Camera::GetProjectionMatrix() const
    {
        Window* window = WindowManager::Get().GetWindow();
        DX_ASSERT(window, "Camera", "Default window not found");

        const float fovY = 74.0f * mathfu::kDegreesToRadians;
        const float aspectRatio = static_cast<float>(window->GetSize().x) / static_cast<float>(window->GetSize().y);
        const float nearPlane = 0.01f;
        const float farPlane = 100.0f;

        return Math::Matrix4x4::Perspective(
            fovY,
            aspectRatio,
            nearPlane,
            farPlane,
            Math::CoordinateSystem::Default);
    }
} // namespace DX
