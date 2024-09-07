#pragma once

#include <RHI/Resource/ResourceEnums.h>

#include <vector>

namespace Vulkan
{
    struct RenderPassDesc
    {
        std::vector<ResourceFormat> m_attachments;
    };
} // namespace Vulkan
