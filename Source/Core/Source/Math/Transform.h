#pragma once

#include <Math/Vector3.h>
#include <Math/Quaternion.h>
#include <Math/Matrix4x4.h>

namespace Math
{
    class Transform
    {
    public:
        static Transform CreateIdentity();
        static Transform CreateFromMatrix(const Matrix4x4& matrix);

        Transform() = default; // Does not initialize members

        Transform(
            const Vector3& position, 
            const Quaternion& rotation = Quaternion::identity, 
            const Vector3& scale = Vector3(1.0f));

        Vector3 GetBasisX() const;
        Vector3 GetBasisY() const;
        Vector3 GetBasisZ() const;

        Matrix4x4 ToMatrix() const;

        Vector3 m_position;
        Quaternion m_rotation;
        Vector3 m_scale;
    };
} // namespace Math
