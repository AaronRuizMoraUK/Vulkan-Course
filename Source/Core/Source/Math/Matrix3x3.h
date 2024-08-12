#pragma once

#include <Math/Vector3.h>

#include "mathfu/matrix.h"

namespace Math
{
    using Matrix3x3 = mathfu::Matrix<float, 3, 3>;

    inline Matrix3x3 CreateMatrix3x3FromBasis(const Vector3& basisX, const Vector3& basisY, const Vector3& basisZ)
    {
        Matrix3x3 rotation = Matrix3x3::Identity();
        rotation.GetColumn(0) = basisX;
        rotation.GetColumn(1) = basisY;
        rotation.GetColumn(2) = basisZ;
        return rotation;
    }
}  // namespace Math
