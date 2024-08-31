#pragma once

#include <RHI/Resource/ResourceEnums.h>
#include <RHI/Resource/Buffer/BufferEnums.h>

namespace Vulkan
{
    struct BufferDesc
    {
        uint32_t m_elementSizeInBytes;
        uint32_t m_elementCount;
        BufferUsageFlags m_usageFlags; // Bitwise operation of BufferUsageFlag
        ResourceMemoryProperty m_memoryProperty;

        const void* m_initialData;
    };
} // namespace Vulkan
