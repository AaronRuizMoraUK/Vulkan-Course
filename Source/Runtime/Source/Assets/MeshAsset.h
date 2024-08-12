#pragma once

#include <Assets/Asset.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Renderer/Vertices.h>

#include <vector>

namespace std::filesystem
{
    class path;
}

namespace DX
{
    struct MeshData
    {
        std::vector<Math::Vector3Packed> m_positions;
        std::vector<Math::Vector2Packed> m_textCoords;
        std::vector<Math::Vector3Packed> m_normals;
        std::vector<Math::Vector3Packed> m_tangents;
        std::vector<Math::Vector3Packed> m_binormals;

        std::vector<Index> m_indices;
    };

    // Mesh asset with the list of vertices, indices and other
    // data needed to create a mesh.
    // 
    // Mesh asset formats supported: fbx and gltf
    class MeshAsset : public Asset<MeshData>
    {
    public:
        // Loads a mesh from a file. The filename is relative to the assets folder.
        static std::shared_ptr<MeshAsset> LoadMeshAsset(const std::string& fileName);

        static inline const AssetType AssetTypeId = 0x73E47A71;

        AssetType GetAssetType() const override
        {
            return AssetTypeId;
        }

    protected:
        friend class AssetManager;
        using Super = Asset<MeshData>;

        MeshAsset(AssetId assetId, std::unique_ptr<MeshData> data);

    private:
        static std::unique_ptr<MeshData> LoadMesh(const std::filesystem::path& fileNamePath);
    };
} // namespace DX
