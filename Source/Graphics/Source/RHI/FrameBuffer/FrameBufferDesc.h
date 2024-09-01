#pragma once

#include <RHI/Resource/ResourceEnums.h>

#include <memory>
#include <vector>

namespace Vulkan
{
    class Image;

    struct FrameBufferDesc
    {
        struct ImageAttachment
        {
            std::shared_ptr<Image> m_image;
            ResourceFormat m_viewFormat;
        };

        using ImageAttachments = std::vector<ImageAttachment>;

        ImageAttachments m_colorAttachments;
        ImageAttachment m_depthStencilAttachment;
    };
} // namespace Vulkan
