#pragma once

#include "mathfu/vector.h"

namespace Math
{
    // Vector4 size might differ depending on platform and build configuration of mathfu (SIMD, padding, etc).
    // To serialize and deserialize as a flat array, use Vector4Packed, which is a POD version of Vector4.
    using Vector4 = mathfu::Vector<float, 4>;
    using Vector4Packed = mathfu::VectorPacked<float, 4>;

    // This is always stored as POD.
    using Vector4Int = mathfu::Vector<int, 4>;

    /// @brief Calculate the dot product of two Vector4.
    ///
    /// @param v1 Vector to multiply
    /// @param v2 Vector to multiply
    /// @return Scalar dot product result.
    inline float Dot(const Vector4& v1, const Vector4& v2) {
        return Vector4::DotProduct(v1, v2);
    }

    /// @brief Normalize an Vector2.
    ///
    /// @param v1 Vector to normalize.
    /// @return Normalized vector.
    inline Vector4 Normalize(const Vector4& v1) {
        return v1.Normalized();
    }

}  // namespace Math
