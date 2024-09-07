#pragma once

#include <RHI/Resource/ResourceEnums.h>
#include <RHI/Resource/ImageView/ImageViewEnums.h>

#include <memory>
#include <vector>

namespace Vulkan
{
    class RenderPass;
    class Image;

    struct ImageAttachment
    {
        std::shared_ptr<Image> m_image;
        ResourceFormat m_viewFormat;
        ImageViewAspectFlags m_viewAspectFlags;
    };

    struct FrameBufferDesc
    {
        RenderPass* m_renderPass = nullptr;

        // Must match 1:1 attachments in Render Pass
        std::vector<ImageAttachment> m_attachments;
    };
} // namespace Vulkan
