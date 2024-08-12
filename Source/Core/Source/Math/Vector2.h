#pragma once

#include "mathfu/vector.h"

namespace Math
{
    // Vector2 size might differ depending on platform and build configuration of mathfu (SIMD, padding, etc).
    // To serialize and deserialize as a flat array, use Vector2Packed, which is a POD version of Vector4.
    using Vector2 = mathfu::Vector<float, 2>;
    using Vector2Packed = mathfu::VectorPacked<float, 2>;

    // This is always stored as POD.
    using Vector2Int = mathfu::Vector<int, 2>;

    /// @brief Calculate the dot product of two Vector2.
    ///
    /// @param v1 Vector to multiply
    /// @param v2 Vector to multiply
    /// @return Scalar dot product result.
    inline float Dot(const Vector2& v1, const Vector2& v2) {
        return Vector2::DotProduct(v1, v2);
    }

    /// @brief Normalize an Vector2.
    ///
    /// @param v1 Vector to normalize.
    /// @return Normalized vector.
    inline Vector2 Normalize(const Vector2& v1) {
        return v1.Normalized();
    }

}  // namespace Math
