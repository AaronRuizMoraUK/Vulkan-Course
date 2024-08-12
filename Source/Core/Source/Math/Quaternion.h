#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix3x3.h>

#include "mathfu/quaternion.h"

namespace Math
{
    using Quaternion = mathfu::Quaternion<float>;

    inline Quaternion CreateQuatFromBasisZ(const Vector3& basisZ, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f))
    {
        const Vector3 basisX = Cross(up, basisZ).Normalized();
        const Vector3 basisY = Cross(basisZ, basisX);

        return Quaternion::FromMatrix(CreateMatrix3x3FromBasis(basisX, basisY, basisZ));
    }
}  // namespace Math
