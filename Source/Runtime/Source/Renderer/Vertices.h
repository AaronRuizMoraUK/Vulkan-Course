#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Color.h>

namespace DX
{
    using Index = uint32_t;

    struct VertexPC
    {
        Math::Vector3Packed m_position;
        Math::ColorPacked m_color;
    };

    struct VertexPUv
    {
        Math::Vector3Packed m_position;
        Math::Vector2Packed m_uv;
    };

    struct VertexPNTBUv
    {
        Math::Vector3Packed m_position;
        Math::Vector3Packed m_normal;
        Math::Vector3Packed m_tangent;
        Math::Vector3Packed m_binormal;
        Math::Vector2Packed m_uv;
    };
} // namespace DX
