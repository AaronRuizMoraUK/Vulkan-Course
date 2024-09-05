#pragma once

#include <stdint.h>

namespace Vulkan
{
    // Bitwise operations on ShaderTypeFlag are allowed.
    enum ShaderTypeFlag
    {
        ShaderType_Vertex = 1 << 0,
        ShaderType_TesselationControl = 1 << 1,
        ShaderType_TesselationEvaluation = 1 << 2,
        ShaderType_Geometry = 1 << 3,
        ShaderType_Fragment = 1 << 4,
        ShaderType_Compute = 1 << 5
    };
    using ShaderTypeFlags = uint32_t;

} // namespace Vulkan
