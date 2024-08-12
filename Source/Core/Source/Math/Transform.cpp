#include <Math/Transform.h>

#include <mathfu/constants.h>

namespace Math
{
    Transform Transform::CreateIdentity()
    {
        return Transform(Vector3(0.0f));
    }

    Transform Transform::CreateFromMatrix(const Matrix4x4& matrix)
    {
        return Transform(
            matrix.TranslationVector3D(),
            Quaternion::FromMatrix(matrix),
            matrix.ScaleVector3D());
    }

    Transform::Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
        : m_position(position)
        , m_rotation(rotation)
        , m_scale(scale)
    {
    }

    Vector3 Transform::GetBasisX() const
    {
        return m_rotation * (mathfu::kAxisX3f * m_scale);
    }

    Vector3 Transform::GetBasisY() const
    {
        return m_rotation * (mathfu::kAxisY3f * m_scale);
    }

    Vector3 Transform::GetBasisZ() const
    {
        return m_rotation * (mathfu::kAxisZ3f * m_scale);
    }

    Matrix4x4 Transform::ToMatrix() const
    {
        return Matrix4x4::Transform(m_position, m_rotation.ToMatrix(), m_scale);
    }
} // namespace Math
