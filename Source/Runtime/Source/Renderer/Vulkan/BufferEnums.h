#pragma once

#include <stdint.h>

namespace Vulkan
{
    // Bitwise operations on BufferUsageFlag are allowed.
    enum BufferUsageFlag
    {
        BufferUsage_VertexBuffer = 1 << 0,
        BufferUsage_IndexBuffer = 1 << 1
    };
    using BufferUsageFlags = uint32_t;

} // namespace Vulkan
