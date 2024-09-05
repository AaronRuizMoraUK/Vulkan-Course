#include <Renderer/Object.h>
#include <Renderer/RendererManager.h>
#include <Assets/TextureAsset.h>
#include <Assets/MeshAsset.h>

#include <RHI/Device/Device.h>
#include <RHI/Resource/Buffer/Buffer.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Resource/ImageView/ImageView.h>
#include <RHI/Sampler/Sampler.h>

#include <Log/Log.h>
#include <Debug/Debug.h>
#include <mathfu/constants.h>

#include <algorithm>

namespace DX
{
    Object::Object() = default;

    Object::~Object() = default;

    std::shared_ptr<Vulkan::ImageView> Object::GetDiffuseImageView() const
    {
        return m_diffuseImageView;
    }

    std::shared_ptr<Vulkan::ImageView> Object::GetEmissiveImageView() const
    {
        return m_emissiveImageView;
    }

    std::shared_ptr<Vulkan::ImageView> Object::GetNormalImageView() const
    {
        return m_normalImageView;
    }

    std::shared_ptr<Vulkan::Sampler> Object::GetSampler() const
    {
        return m_imageSampler;
    }

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

        // Diffuse Texture
        {
            auto textureAsset = TextureAsset::LoadTextureAsset(m_diffuseFilename);
            DX_ASSERT(textureAsset.get(), "Object", "Failed to load texture");

            Vulkan::ImageDesc imageDesc = {};
            imageDesc.m_imageType = Vulkan::ImageType::Image2D;
            imageDesc.m_dimensions = Math::Vector3Int(textureAsset->GetData()->m_size, 1);
            imageDesc.m_mipCount = 1;
            imageDesc.m_format = Vulkan::ResourceFormat::R8G8B8A8_UNORM;
            imageDesc.m_tiling = Vulkan::ImageTiling::Optimal;
            imageDesc.m_usageFlags = Vulkan::ImageUsage_Sampled;
            imageDesc.m_initialData = textureAsset->GetData()->m_data;

            m_diffuseImage = std::make_shared<Vulkan::Image>(renderer->GetDevice(), imageDesc);
            if (!m_diffuseImage->Initialize())
            {
                DX_LOG(Fatal, "Object", "Failed to create diffuse image.");
                return;
            }

            Vulkan::ImageViewDesc imageViewDesc = {};
            imageViewDesc.m_image = m_diffuseImage;
            imageViewDesc.m_viewFormat = m_diffuseImage->GetImageDesc().m_format;
            imageViewDesc.m_aspectFlags = Vulkan::ImageViewAspect_Color;
            imageViewDesc.m_firstMip = 0;
            imageViewDesc.m_mipCount = 0;

            m_diffuseImageView = std::make_shared<Vulkan::ImageView>(renderer->GetDevice(), imageViewDesc);
            if (!m_diffuseImageView->Initialize())
            {
                DX_LOG(Fatal, "Object", "Failed to create diffuse image view.");
                return;
            }
        }

        // Emissive Texture
        {
            if (m_emissiveFilename.empty())
            {
                uint32_t imageData = 0; // 1 texel with all RGBA set to zero

                Vulkan::ImageDesc imageDesc = {};
                imageDesc.m_imageType = Vulkan::ImageType::Image2D;
                imageDesc.m_dimensions = Math::Vector3Int(1, 1, 1);
                imageDesc.m_mipCount = 1;
                imageDesc.m_format = Vulkan::ResourceFormat::R8G8B8A8_UNORM;
                imageDesc.m_tiling = Vulkan::ImageTiling::Optimal;
                imageDesc.m_usageFlags = Vulkan::ImageUsage_Sampled;
                imageDesc.m_initialData = &imageData;

                m_emissiveImage = std::make_shared<Vulkan::Image>(renderer->GetDevice(), imageDesc);
                if (!m_emissiveImage->Initialize())
                {
                    DX_LOG(Fatal, "Object", "Failed to create mock emissive image.");
                    return;
                }
            }
            else
            {
                auto textureAsset = TextureAsset::LoadTextureAsset(m_emissiveFilename);
                DX_ASSERT(textureAsset.get(), "Object", "Failed to load texture");

                Vulkan::ImageDesc imageDesc = {};
                imageDesc.m_imageType = Vulkan::ImageType::Image2D;
                imageDesc.m_dimensions = Math::Vector3Int(textureAsset->GetData()->m_size, 1);
                imageDesc.m_mipCount = 1;
                imageDesc.m_format = Vulkan::ResourceFormat::R8G8B8A8_UNORM;
                imageDesc.m_tiling = Vulkan::ImageTiling::Optimal;
                imageDesc.m_usageFlags = Vulkan::ImageUsage_Sampled;
                imageDesc.m_initialData = textureAsset->GetData()->m_data;

                m_emissiveImage = std::make_shared<Vulkan::Image>(renderer->GetDevice(), imageDesc);
                if (!m_emissiveImage->Initialize())
                {
                    DX_LOG(Fatal, "Object", "Failed to create emissive image.");
                    return;
                }
            }

            Vulkan::ImageViewDesc imageViewDesc = {};
            imageViewDesc.m_image = m_diffuseImage;
            imageViewDesc.m_viewFormat = m_diffuseImage->GetImageDesc().m_format;
            imageViewDesc.m_aspectFlags = Vulkan::ImageViewAspect_Color;
            imageViewDesc.m_firstMip = 0;
            imageViewDesc.m_mipCount = 0;

            m_emissiveImageView = std::make_shared<Vulkan::ImageView>(renderer->GetDevice(), imageViewDesc);
            if (!m_emissiveImageView->Initialize())
            {
                DX_LOG(Fatal, "Object", "Failed to create emissive image view.");
                return;
            }
        }

        // Normal Texture
        {
            auto textureAsset = TextureAsset::LoadTextureAsset(m_normalFilename);
            DX_ASSERT(textureAsset.get(), "Object", "Failed to load texture");

            Vulkan::ImageDesc imageDesc = {};
            imageDesc.m_imageType = Vulkan::ImageType::Image2D;
            imageDesc.m_dimensions = Math::Vector3Int(textureAsset->GetData()->m_size, 1);
            imageDesc.m_mipCount = 1;
            imageDesc.m_format = Vulkan::ResourceFormat::R8G8B8A8_UNORM;
            imageDesc.m_tiling = Vulkan::ImageTiling::Optimal;
            imageDesc.m_usageFlags = Vulkan::ImageUsage_Sampled;
            imageDesc.m_initialData = textureAsset->GetData()->m_data;

            m_normalImage = std::make_shared<Vulkan::Image>(renderer->GetDevice(), imageDesc);
            if (!m_normalImage->Initialize())
            {
                DX_LOG(Fatal, "Object", "Failed to create normal image.");
                return;
            }

            Vulkan::ImageViewDesc imageViewDesc = {};
            imageViewDesc.m_image = m_diffuseImage;
            imageViewDesc.m_viewFormat = m_diffuseImage->GetImageDesc().m_format;
            imageViewDesc.m_aspectFlags = Vulkan::ImageViewAspect_Color;
            imageViewDesc.m_firstMip = 0;
            imageViewDesc.m_mipCount = 0;

            m_normalImageView = std::make_shared<Vulkan::ImageView>(renderer->GetDevice(), imageViewDesc);
            if (!m_normalImageView->Initialize())
            {
                DX_LOG(Fatal, "Object", "Failed to create normal image view.");
                return;
            }
        }

        // Sampler State
        {
            Vulkan::SamplerDesc samplerDesc = {};
            samplerDesc.m_minFilter = Vulkan::FilterSampling::Linear;
            samplerDesc.m_magFilter = Vulkan::FilterSampling::Linear;
            samplerDesc.m_mipFilter = Vulkan::FilterSampling::Linear;
            samplerDesc.m_addressU = Vulkan::AddressMode::Wrap;
            samplerDesc.m_addressV = Vulkan::AddressMode::Wrap;
            samplerDesc.m_addressW = Vulkan::AddressMode::Wrap;
            samplerDesc.m_mipBias = 0.0f;
            samplerDesc.m_mipClamp = Vulkan::NoMipClamping;
            samplerDesc.m_maxAnisotropy = 1.0f;

            m_imageSampler = std::make_shared<Vulkan::Sampler>(renderer->GetDevice(), samplerDesc);
            if (!m_imageSampler->Initialize())
            {
                DX_LOG(Fatal, "Object", "Failed to create image sampler.");
                return;
            }
        }
    }

    Cube::Cube(const Math::Transform& transform,
        const Math::Vector3& extends)
    {
        m_transform = transform;
        m_diffuseFilename = "Textures/Wall_Stone_Albedo.png";
        m_normalFilename = "Textures/Wall_Stone_Normal.png";

        const Math::Vector3 half = 0.5f * extends;

        // 6 faces, 2 triangles each face, 3 vertices each triangle.
        // Clockwise order (CW) - LeftHand

        /*
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
        */

        m_vertexData =
        {
            // Front face
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), /*Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), /*Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), /*Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), /*Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({1.0f, 0.0f}) },

            // Back face
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), /*Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), /*Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), /*Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), /*Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({1.0f, 0.0f}) },

            // Right face
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), /*Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), /*Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), /*Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), /*Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({1.0f, 0.0f}) },

            // Left face
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), /*Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), /*Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), /*Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), /*Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f), Math::Vector3Packed(-mathfu::kAxisY3f),*/ Math::Vector2Packed({1.0f, 0.0f}) },

            // Top face
            { Math::Vector3Packed({-half.x,  half.y, -half.z}), /*Math::Vector3Packed(mathfu::kAxisY3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f),*/ Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({-half.x,  half.y,  half.z}), /*Math::Vector3Packed(mathfu::kAxisY3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f),*/ Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x,  half.y,  half.z}), /*Math::Vector3Packed(mathfu::kAxisY3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f),*/ Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({ half.x,  half.y, -half.z}), /*Math::Vector3Packed(mathfu::kAxisY3f), Math::Vector3Packed(mathfu::kAxisX3f), Math::Vector3Packed(-mathfu::kAxisZ3f),*/ Math::Vector2Packed({1.0f, 0.0f}) },

            // Bottom face
            { Math::Vector3Packed({ half.x, -half.y,  half.z}), /*Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f),*/ Math::Vector2Packed({0.0f, 0.0f}) },
            { Math::Vector3Packed({ half.x, -half.y, -half.z}), /*Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f),*/ Math::Vector2Packed({0.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x, -half.y, -half.z}), /*Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f),*/ Math::Vector2Packed({1.0f, 1.0f}) },
            { Math::Vector3Packed({-half.x, -half.y,  half.z}), /*Math::Vector3Packed(-mathfu::kAxisY3f), Math::Vector3Packed(-mathfu::kAxisX3f), Math::Vector3Packed(mathfu::kAxisZ3f),*/ Math::Vector2Packed({1.0f, 0.0f}) },
        };

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

    Mesh::Mesh(const Math::Transform& transform,
        const std::string& meshFilename,
        const std::string& diffuseFilename,
        const std::string& normalFilename,
        const std::string& emissiveFilename)
    {
        m_transform = transform;
        m_diffuseFilename = diffuseFilename;
        m_normalFilename = normalFilename;
        m_emissiveFilename = emissiveFilename;

        auto meshAsset = MeshAsset::LoadMeshAsset(meshFilename);
        if (!meshAsset)
        {
            DX_LOG(Fatal, "Mesh", "Failed to load mesh asset %s", meshFilename.c_str());
            return;
        }

        const MeshData* meshData = meshAsset->GetData();

        m_vertexData.resize(meshData->m_positions.size());
        for (uint32_t i = 0; i < meshData->m_positions.size(); ++i)
        {
            m_vertexData[i] = VertexPUv
            {
                .m_position = meshData->m_positions[i],
                //.m_normal = meshData->m_normals[i],
                //.m_tangent = meshData->m_tangents[i],
                //.m_binormal = meshData->m_binormals[i],
                .m_uv = meshData->m_textCoords[i]
            };
        }

        m_indexData = meshData->m_indices;

        CreateBuffers();
    }
} // namespace DX
