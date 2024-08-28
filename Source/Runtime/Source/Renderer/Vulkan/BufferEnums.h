#pragma once

#include <stdint.h>

namespace Vulkan
{
    // Bitwise operations on BufferUsageFlag are allowed.
    enum BufferUsageFlag
    {
        BufferUsage_VertexBuffer = 1 << 0,
        BufferUsage_IndexBuffer = 1 << 1,
        BufferUsage_UniformBuffer = 1 << 2,
    };
    using BufferUsageFlags = uint32_t;

    enum class BufferMemoryProperty
    {
        Unknown = 0,

        HostVisible, // Visible by CPU. Suitable for data that needs to be updated regularly. Non-optimal for GPU performance.
        DeviceLocal, // Accessible by GPU only. Data set during buffer creation. Optimal for GPU performance.

        Count
    };
} // namespace Vulkan
