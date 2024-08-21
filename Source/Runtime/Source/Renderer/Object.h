#pragma once

#include <Math/Transform.h>
#include <Renderer/Vertices.h>

#include <vector>
#include <memory>

namespace Vulkan
{
    class Buffer;
}

namespace DX
{
    class Object
    {
    public:
        Object();
        virtual ~Object() = 0;

        uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_indexData.size()); }

        Math::Transform& GetTransform() { return m_transform; }
        const Math::Transform& GetTransform() const { return m_transform; }
        void SetTransform(const Math::Transform& transform) { m_transform = transform; }

        std::shared_ptr<Vulkan::Buffer> GetVertexBuffer() const;
        std::shared_ptr<Vulkan::Buffer> GetIndexBuffer() const;

    protected:
        void CreateBuffers();

        uint32_t GetVertexSize() const { return sizeof(VertexPC); }
        uint32_t GetIndexSize() const { return sizeof(Index); }

        Math::Transform m_transform = Math::Transform::CreateIdentity();

        // Filled by subclass
        std::vector<VertexPC> m_vertexData;
        std::vector<Index> m_indexData;

    private:
        std::shared_ptr<Vulkan::Buffer> m_vertexBuffer;
        std::shared_ptr<Vulkan::Buffer> m_indexBuffer;
    };

    class SimpleObject : public Object
    {
    public:
        SimpleObject(const Math::Transform& transform,
            const std::vector<VertexPC>& vertexData,
            const std::vector<Index> indexData);
    };

    class Cube : public Object
    {
    public:
        Cube(const Math::Transform& transform, 
            const Math::Vector3& extends);
    };
} // namespace DX
