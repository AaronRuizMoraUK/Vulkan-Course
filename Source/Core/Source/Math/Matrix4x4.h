#pragma once

#include "mathfu/matrix.h"
#include "Math/Vector4.h"

namespace Math
{
    // Handedness values used in mathfu.
    struct CoordinateSystem
    {
        // Right Hand
        //      Y
        //      | 
        //      | 
        //      |
        //      -----> X
        //     /
        //    /
        //   Z 
        static inline const float RightHand = 1.0f;

        // Left Hand
        //      Y
        //      |  Z
        //      | /
        //      |/
        //      -----> X
        static inline const float LeftHand = -1.0f;

        // Default handedness to use
        static inline const float Default = LeftHand;
    };

    // Reference Systems and Transformations using Matrix4x4.
    // 
    // Matrix4x4 is column major and its helpers will create matrices the following way:
    // 
    //        Column0 Column1  Column2  Column3
    // Row0 |  AxisX    AxisY   AxisZ    PosX |
    // Row1 |  AxisX    AxisY   AxisZ    PosY |
    // Row2 |  AxisX    AxisY   AxisZ    PosZ |
    // Row2 |    0       0        0       1   |
    //
    // Example accessing elements: M[row][col] -> M[1][3] = PosY
    //
    // Column major transformation order reads left to right:
    //
    // TransformedPoint = TransformMatrix * Point  (Vector4)
    // TransformedVector = TransformMatrix * Vector (Vector3)
    // TransformedVertex = ProjectionMatrix * ViewMatrix * WorldMatrix * Vertex (Vector4)
    //
    // Internal memory layout:
    // 
    // Column0 (AxisX,0)
    // Column1 (AxisY,0)
    // Column2 (AxisZ,0)
    // Column3 (Pos,1)
    // 
    // HLSL matrices (float4x4) are also column major by default.

    using Matrix4x4 = mathfu::Matrix<float, 4, 4>;

    // Packed version of Matrix4x4
    struct Matrix4x4Packed
    {
        Matrix4x4Packed() = default;

        Matrix4x4Packed(const Matrix4x4& matrix)
        {
            matrix.Pack(columns);
        }

        Matrix4x4Packed& operator=(const Matrix4x4& matrix) {
            matrix.Pack(columns);
            return *this;
        }

        Vector4Packed columns[4];
    };
}  // namespace Math
