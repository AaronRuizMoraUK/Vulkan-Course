#pragma once

#include <Renderer/Vulkan/BufferEnums.h>

namespace Vulkan
{
    struct BufferDesc
    {
        uint32_t m_elementSizeInBytes;
        uint32_t m_elementCount;
        BufferUsageFlags m_usageFlags; // Bitwise operation of BufferUsageFlag
        BufferMemoryProperty m_memoryProperty;

        const void* m_initialData;
    };
} // namespace Vulkan
