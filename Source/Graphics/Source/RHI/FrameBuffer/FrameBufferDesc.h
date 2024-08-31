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

        // When true, the 2D depth stencil attachment will be created.
        // It'll have the same dimensions as the first color attachment
        // and format D32_SFLOAT_S8_UINT or D24_UNORM_S8_UINT.
        bool m_createDepthStencilAttachment;
    };
} // namespace Vulkan
