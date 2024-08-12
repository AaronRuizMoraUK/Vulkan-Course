#include <Assets/MeshAsset.h>
#include <Assets/AssetManager.h>
#include <Log/Log.h>
#include <Debug/Debug.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace DX
{
    namespace Internal
    {
        bool ProcessAssimpMesh(MeshData* meshData, const aiMesh* mesh, const aiMatrix4x4& transform)
        {
            uint32_t vertexBaseCount = static_cast<uint32_t>(meshData->m_positions.size());
            uint32_t indexBaseCount = static_cast<uint32_t>(meshData->m_indices.size());

            const aiMatrix3x3 transform3x3(transform);

            if (mesh->HasPositions())
            {
                meshData->m_positions.resize(vertexBaseCount + mesh->mNumVertices);
                std::transform(mesh->mVertices, mesh->mVertices + mesh->mNumVertices,
                    meshData->m_positions.begin() + vertexBaseCount,
                    [&transform](const aiVector3D& lhs)
                    {
                        const aiVector3D lhsTransformed = transform * lhs;

                        Math::Vector3Packed rhs;
                        rhs.x = lhsTransformed.x;
                        rhs.y = lhsTransformed.y;
                        rhs.z = lhsTransformed.z;
                        return rhs;
                    });

                meshData->m_indices.resize(indexBaseCount + mesh->mNumFaces * 3);
                for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
                {
                    DX_ASSERT(mesh->mFaces[faceIndex].mNumIndices == 3, "MeshAsset", "Mesh face must have 3 indices");

                    const uint32_t index = indexBaseCount + faceIndex * 3;
                    meshData->m_indices[index + 0] = vertexBaseCount + mesh->mFaces[faceIndex].mIndices[0];
                    meshData->m_indices[index + 1] = vertexBaseCount + mesh->mFaces[faceIndex].mIndices[1];
                    meshData->m_indices[index + 2] = vertexBaseCount + mesh->mFaces[faceIndex].mIndices[2];
                }
            }
            else
            {
                DX_LOG(Error, "MeshAsset", "Mesh %s has no positions\n", mesh->mName.C_Str());
                return false;
            }

            // Use first set of texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                meshData->m_textCoords.resize(vertexBaseCount + mesh->mNumVertices);
                std::transform(mesh->mTextureCoords[0], mesh->mTextureCoords[0] + mesh->mNumVertices,
                    meshData->m_textCoords.begin() + vertexBaseCount,
                    [](const aiVector3D& lhs)
                    {
                        Math::Vector2Packed rhs;
                        rhs.x = lhs.x;
                        rhs.y = lhs.y;
                        return rhs;
                    });
            }
            else
            {
                DX_LOG(Error, "MeshAsset", "Mesh %s has no texture coordinates\n", mesh->mName.C_Str());
                return false;
            }

            if (mesh->HasNormals())
            {
                meshData->m_normals.resize(vertexBaseCount + mesh->mNumVertices);
                std::transform(mesh->mNormals, mesh->mNormals + mesh->mNumVertices,
                    meshData->m_normals.begin() + vertexBaseCount,
                    [&transform3x3](const aiVector3D& lhs)
                    {
                        const aiVector3D lhsTransformed = transform3x3 * lhs;

                        Math::Vector3Packed rhs;
                        rhs.x = lhsTransformed.x;
                        rhs.y = lhsTransformed.y;
                        rhs.z = lhsTransformed.z;
                        return rhs;
                    });
            }
            else
            {
                DX_LOG(Error, "MeshAsset", "Mesh %s has no normals\n", mesh->mName.C_Str());
                return false;
            }

            if (mesh->HasTangentsAndBitangents())
            {
                meshData->m_tangents.resize(vertexBaseCount + mesh->mNumVertices);
                std::transform(mesh->mTangents, mesh->mTangents + mesh->mNumVertices,
                    meshData->m_tangents.begin() + vertexBaseCount,
                    [&transform3x3](const aiVector3D& lhs)
                    {
                        const aiVector3D lhsTransformed = transform3x3 * lhs;

                        Math::Vector3Packed rhs;
                        rhs.x = lhsTransformed.x;
                        rhs.y = lhsTransformed.y;
                        rhs.z = lhsTransformed.z;
                        return rhs;
                    });

                meshData->m_binormals.resize(vertexBaseCount + mesh->mNumVertices);
                std::transform(mesh->mBitangents, mesh->mBitangents + mesh->mNumVertices,
                    meshData->m_binormals.begin() + vertexBaseCount,
                    [&transform3x3](const aiVector3D& lhs)
                    {
                        const aiVector3D lhsTransformed = transform3x3 * lhs;

                        Math::Vector3Packed rhs;
                        rhs.x = lhsTransformed.x;
                        rhs.y = lhsTransformed.y;
                        rhs.z = lhsTransformed.z;
                        return rhs;
                    });
            }
            else
            {
                DX_LOG(Error, "MeshAsset", "Mesh %s has no tangents and binormals\n", mesh->mName.C_Str());
                return false;
            }

            return true;
        }

        bool ProcessAssimpNode(MeshData* meshData, const aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform)
        {
            // Calculate the node's model transformation
            const aiMatrix4x4 nodeModelTransform = parentTransform * node->mTransformation;

            // Process each mesh located at this node
            for (uint32_t i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                if (!ProcessAssimpMesh(meshData, mesh, nodeModelTransform))
                {
                    return false;
                }
            }

            // Recursively process each child node
            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                if (!ProcessAssimpNode(meshData, node->mChildren[i], scene, nodeModelTransform))
                {
                    return false;
                }
            }

            return true;
        }
    }

    MeshAsset::MeshAsset(AssetId assetId, std::unique_ptr<MeshData> data)
        : Super(assetId, std::move(data))
    {
    }

    std::shared_ptr<MeshAsset> MeshAsset::LoadMeshAsset(const std::string& fileName)
    {
        return DX::AssetManager::Get().LoadAssetAs<MeshAsset>(
            fileName, 
            std::bind(&MeshAsset::LoadMesh, std::placeholders::_1));
    }

    std::unique_ptr<MeshData> MeshAsset::LoadMesh(const std::filesystem::path& fileNamePath)
    {
        Assimp::Importer importer;

        const uint32_t importerFlags = 
            aiProcess_Triangulate |
            aiProcess_ConvertToLeftHanded |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices;

        const aiScene* scene = importer.ReadFile(fileNamePath.generic_string(), importerFlags);

        if (!scene || 
            !scene->mRootNode ||
            scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        {
            DX_LOG(Error, "MeshAsset", "Assimp failed to import mesh: %s\n\nError message: %s\n", 
                fileNamePath.generic_string().c_str(), importer.GetErrorString());
            return nullptr;
        }

        if (!scene->HasMeshes())
        {
            DX_LOG(Error, "MeshAsset", "Assimp failed to import mesh: %s\n\nError message: %s\n",
                fileNamePath.generic_string().c_str(), importer.GetErrorString());
            return nullptr;
        }

        auto meshData = std::make_unique<MeshData>();

        // TODO: Import AABBs and separate sort mesh data in sub-meshes.

        if (aiMatrix4x4 identityMatrix;
            !Internal::ProcessAssimpNode(meshData.get(), scene->mRootNode, scene, identityMatrix))
        {
            DX_LOG(Error, "MeshAsset", "Assimp failed to process mesh: %s",
                fileNamePath.generic_string().c_str());
            return nullptr;
        }

        return meshData;
    }
} // namespace DX
