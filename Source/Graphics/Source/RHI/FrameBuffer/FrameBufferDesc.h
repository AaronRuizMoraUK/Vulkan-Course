#pragma once

#include <RHI/Resource/ResourceEnums.h>

#include <memory>
#include <vector>

namespace Vulkan
{
    class RenderPass;
    class Image;

    struct FrameBufferDesc
    {
        RenderPass* m_renderPass = nullptr;

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
