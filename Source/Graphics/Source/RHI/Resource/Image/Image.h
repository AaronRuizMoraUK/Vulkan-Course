#pragma once

#include <Math/Vector2.h>

typedef struct VkImage_T* VkImage;

namespace Vulkan
{
    struct Image
    {
        VkImage m_vkImage = nullptr;
        int m_vkFormat = -1;
        Math::Vector2Int m_size = Math::Vector2Int(0, 0);
    };
} // namespace Vulkan
