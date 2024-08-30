#pragma once

#include <stdint.h>

namespace Vulkan
{
    enum ShaderType
    {
        ShaderType_Unknown = 0,

        ShaderType_Vertex,
        ShaderType_TesselationControl,
        ShaderType_TesselationEvaluation,
        ShaderType_Geometry,
        ShaderType_Fragment,
        ShaderType_Compute,

        ShaderType_Count
    };
} // namespace Vulkan
