#pragma once

#include "mathfu/vector.h"

namespace Math
{
    // Vector3 size might differ depending on platform and build configuration of mathfu (SIMD, padding, etc).
    // To serialize and deserialize as a flat array, use Vector3Packed, which is a POD version of Vector3.
    using Vector3 = mathfu::Vector<float, 3>;
    using Vector3Packed = mathfu::VectorPacked<float, 3>;

    // This is always stored as POD.
    using Vector3Int = mathfu::Vector<int, 3>;

    /// @brief Calculate the cross product of two Vector3.
    ///
    /// @param v1 Vector to multiply
    /// @param v2 Vector to multiply
    /// @return 3-dimensional vector that contains the result.
    inline Vector3 Cross(const Vector3& v1, const Vector3& v2) {
        return Vector3::CrossProduct(v1, v2);
    }

    /// @brief Calculate the dot product of two Vector3.
    ///
    /// @param v1 Vector to multiply
    /// @param v2 Vector to multiply
    /// @return Scalar dot product result.
    inline float Dot(const Vector3& v1, const Vector3& v2) {
        return Vector3::DotProduct(v1, v2);
    }

    /// @brief Normalize an Vector3.
    ///
    /// @param v1 Vector to normalize.
    /// @return Normalized vector.
    inline Vector3 Normalize(const Vector3& v1) {
        return v1.Normalized();
    }

}  // namespace Math
