#pragma once

#include <Math/Transform.h>
#include <Renderer/Vertices.h>

#include <vector>
#include <memory>
#include <string>

namespace Vulkan
{
    class Buffer;
    class Image;
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

        uint32_t GetVertexSize() const { return sizeof(VertexPUv); }
        uint32_t GetIndexSize() const { return sizeof(Index); }

        Math::Transform m_transform = Math::Transform::CreateIdentity();

        // Filled by subclass
        std::vector<VertexPUv> m_vertexData;
        std::vector<Index> m_indexData;

        // Filled by subclass
        std::string m_diffuseFilename;
        std::string m_emissiveFilename;
        std::string m_normalFilename;

    private:
        std::shared_ptr<Vulkan::Buffer> m_vertexBuffer;
        std::shared_ptr<Vulkan::Buffer> m_indexBuffer;

        std::shared_ptr<Vulkan::Image> m_diffuseTexture;
        std::shared_ptr<Vulkan::Image> m_emissiveTexture;
        std::shared_ptr<Vulkan::Image> m_normalTexture;
    };

    class Cube : public Object
    {
    public:
        Cube(const Math::Transform& transform, 
            const Math::Vector3& extends);
    };

    class Mesh : public Object
    {
    public:
        Mesh(const Math::Transform& transform,
            const std::string& meshFilename,
            const std::string& diffuseFilename,
            const std::string& normalFilename,
            const std::string& emissiveFilename = "");
    };
} // namespace DX
