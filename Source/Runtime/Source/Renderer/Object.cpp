#include <Renderer/Object.h>
#include <Renderer/RendererManager.h>

#include <RHI/Device/Device.h>
#include <RHI/Resource/Buffer/Buffer.h>

#include <Log/Log.h>
#include <Debug/Debug.h>
#include <mathfu/constants.h>

#include <algorithm>

namespace DX
{
    Object::Object() = default;

    Object::~Object() = default;

    std::shared_ptr<Vulkan::Buffer> Object::GetVertexBuffer() const
    {
        return m_vertexBuffer;
    }

    std::shared_ptr<Vulkan::Buffer> Object::GetIndexBuffer() const
    {
        return m_indexBuffer;
    }

    void Object::CreateBuffers()
    {
        auto* renderer = RendererManager::Get().GetRenderer();
        DX_ASSERT(renderer, "Object", "Default renderer not found");

        // Vertex Buffer
        {
            Vulkan::BufferDesc vertexBufferDesc = {};
            vertexBufferDesc.m_elementSizeInBytes = GetVertexSize();
            vertexBufferDesc.m_elementCount = static_cast<uint32_t>(m_vertexData.size());
            vertexBufferDesc.m_usageFlags = Vulkan::BufferUsage_VertexBuffer;
            vertexBufferDesc.m_memoryProperty = Vulkan::ResourceMemoryProperty::DeviceLocal;
            vertexBufferDesc.m_initialData = m_vertexData.data();

            m_vertexBuffer = std::make_shared<Vulkan::Buffer>(renderer->GetDevice(), vertexBufferDesc);
            if (!m_vertexBuffer->Initialize())
            {
                DX_LOG(Fatal, "Object", "Failed to create vertex buffer.");
                return;
            }
        }

        // Index Buffer
        {
            Vulkan::BufferDesc indexBufferDesc = {};
            indexBufferDesc.m_elementSizeInBytes = GetIndexSize();
            indexBufferDesc.m_elementCount = static_cast<uint32_t>(m_indexData.size());
            indexBufferDesc.m_usageFlags = Vulkan::BufferUsage_IndexBuffer;
            indexBufferDesc.m_memoryProperty = Vulkan::ResourceMemoryProperty::DeviceLocal;
            indexBufferDesc.m_initialData = m_indexData.data();

            m_indexBuffer = std::make_shared<Vulkan::Buffer>(renderer->GetDevice(), indexBufferDesc);
            if (!m_indexBuffer->Initialize())
            {
                DX_LOG(Fatal, "Object", "Failed to create index buffer.");
                return;
            }
        }
    }

    SimpleObject::SimpleObject(const Math::Transform& transform,
        const std::vector<VertexPC>& vertexData,
        const std::vector<Index> indexData)
    {
        m_transform = transform;
        m_vertexData = vertexData;
        m_indexData = indexData;

        CreateBuffers();
    }

    Cube::Cube(const Math::Transform& transform,
        const Math::Vector3& extends)
    {
        m_transform = transform;

        const Math::Vector3 half = 0.5f * extends;

        // 6 faces, 2 triangles each face, 3 vertices each triangle.
        // Clockwise order (CW) - LeftHand

        m_vertexData =
        {
            // Front face
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), Math::ColorPacked(Math::Colors::Red) },
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), Math::ColorPacked(Math::Colors::Red) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), Math::ColorPacked(Math::Colors::Red) },
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), Math::ColorPacked(Math::Colors::Red) },

            // Back face 
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), Math::ColorPacked(Math::Colors::Green) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), Math::ColorPacked(Math::Colors::Green) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), Math::ColorPacked(Math::Colors::Green) },
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), Math::ColorPacked(Math::Colors::Green) },

            // Right face
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), Math::ColorPacked(Math::Colors::Blue) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), Math::ColorPacked(Math::Colors::Blue) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), Math::ColorPacked(Math::Colors::Blue) },
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), Math::ColorPacked(Math::Colors::Blue) },

            // Left face
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), Math::ColorPacked(Math::Colors::Yellow) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), Math::ColorPacked(Math::Colors::Yellow) },
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), Math::ColorPacked(Math::Colors::Yellow) },
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), Math::ColorPacked(Math::Colors::Yellow) },

            // Top face
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), Math::ColorPacked(Math::Colors::Orange) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), Math::ColorPacked(Math::Colors::Orange) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), Math::ColorPacked(Math::Colors::Orange) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), Math::ColorPacked(Math::Colors::Orange) },

            // Bottom face
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), Math::ColorPacked(Math::Colors::Purple) },
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), Math::ColorPacked(Math::Colors::Purple) },
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), Math::ColorPacked(Math::Colors::Purple) },
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), Math::ColorPacked(Math::Colors::Purple) },
        };

        /*
        m_vertexData =
        {
            // Front face
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({1.0f, 0.0f}) },

            // Back face
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({1.0f, 0.0f}) },

            // Right face
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({1.0f, 0.0f}) },

            // Left face
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector2Packed({1.0f, 0.0f}) },

            // Top face
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), Math::Vector3Packed(mathfu::kAxisY3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), Math::Vector3Packed(mathfu::kAxisY3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), Math::Vector3Packed(mathfu::kAxisY3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), Math::Vector3Packed(mathfu::kAxisY3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector2Packed({1.0f, 0.0f}) },

            // Bottom face
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector2Packed({1.0f, 0.0f}) },
        };
        */

        m_indexData =
        {
            // Front face
            0, 1, 2,
            2, 3, 0,

            // Back face
            4, 5, 6,
            6, 7, 4,

            // Right face
            8, 9, 10,
            10, 11, 8,

            // Left face
            12, 13, 14,
            14, 15, 12,

            // Top face
            16, 17, 18,
            18, 19, 16,

            // Top face
            20, 23, 22,
            22, 21, 20
        };

        CreateBuffers();
    }
} // namespace DX
