#include <RHI/RenderPass/RenderPass.h>

#include <RHI/Device/Device.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    RenderPass::RenderPass(Device* device, ResourceFormat colorFormat, ResourceFormat depthStencilFormat)
        : m_device(device)
        , m_colorFormat(colorFormat)
        , m_depthStencilFormat(depthStencilFormat)
    {
    }

    RenderPass::~RenderPass()
    {
        Terminate();
    }

    bool RenderPass::Initialize()
    {
        if (m_vkRenderPass)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan RenderPass", "Initializing Vulkan RenderPass...");

        if (!CreateVkRenderPass())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void RenderPass::Terminate()
    {
        DX_LOG(Info, "Vulkan RenderPass", "Terminating Vulkan RenderPass...");

        vkDestroyRenderPass(m_device->GetVkDevice(), m_vkRenderPass, nullptr);
        m_vkRenderPass = nullptr;
    }

    VkRenderPass RenderPass::GetVkRenderPass()
    {
        return m_vkRenderPass;
    }

    bool RenderPass::CreateVkRenderPass()
    {
        // TODO: Client code needs to be able to configure all this from a RenderPassDesc structure
        //       and not being hard-coded here.

        // About image layouts in attachments
        // 
        // Frame buffer data will be stored as an image, but images can be given
        // different data layouts to give optimal use for certain operations (read, write, present, etc).
        //
        // The layouts are specified in the attachments assigned to render passes and subpasses,
        // and they indicate the layout the image has to be and the layout the image has changed to 
        // when the passes and subpasses are being executed.
        //
        // In our responsibility to specify the correct image layouts for the images while they
        // are being used by render passes and subpasses.

        // Attachments of the render pass
        const std::vector<VkAttachmentDescription> attachments = {
            // 0) Color Attachment
            {
                .flags = 0,
                .format = ToVkFormat(m_colorFormat), // Format to use for attachment
                .samples = VK_SAMPLE_COUNT_1_BIT, // Number of samples to write for MSAA

                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // What to do with attachment before rendering
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // What to do with attachment after rendering
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // Image data layout expected before render pass starts
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // Image data layout to convert to after render pass finishes
            },
            // 1) Depth/Stencil Attachment
            {
                .flags = 0,
                .format = ToVkFormat(m_depthStencilFormat),
                .samples = VK_SAMPLE_COUNT_1_BIT,

                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            }
        };

        // Render subpass
        // 
        // A subpass has references to Render Pass's attachment descriptors (vkRenderPassCreateInfo.pAttachments),
        // not the attachment descriptors themselves. The reference is specify with an Attachment Reference, where
        // indices to vkRenderPassCreateInfo.pAttachments are specified.
        //
        // NOTE: A subpass doesn't have to use all attachments defined in the render pass.
        //       The render pass defines them all, the subpasses indicates which ones are used.
        const std::vector<VkAttachmentReference> colorAttachmentReferences = {
            {
                .attachment = 0, // Index of the attachment inside vkRenderPassCreateInfo.pAttachments
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // Image data layout to convert to before render subpass starts
            }
        };
        const VkAttachmentReference depthStencilAttachmentReference = {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        const VkSubpassDescription subpass = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, // What pipeline the subpass is to be bound to
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size()),
            .pColorAttachments = colorAttachmentReferences.data(),
            .pResolveAttachments = 0,
            .pDepthStencilAttachment = &depthStencilAttachmentReference,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        // Subpass dependency
        // 
        // A subpass dependency is needed to determine when layout transitions occur.
        // It does implicit layout transitions, the attachments have WHAT layouts
        // to have, but with the subpass dependency we indicate WHEN we want the
        // layout transition operation to occur.
        //
        // It's our responsibility to specify the right points within the render pass when the layout
        // transition can start and when it needs to be finished. For example, after subpass A
        // finishes you can start the transition and be done before this subpass B starts.
        // Notice we haven't say explicitly when the operation happens, but indicated a range in time
        // when the GPU will need to do the operation.
        //
        // In the subpass dependency we specify not only between which subpasses the operation need
        // to happen, but we also specify at what stage inside the subpass' pipeline can the operation start
        // and expected to finish. For example, start after Vertex Shader of subpass A and finish before
        // Fragment Shader of subpass B.
        //
        // Special keywords to be aware of:
        // - Subpass index VK_SUBPASS_EXTERNAL: It means "anything that takes place outside our subpasses".
        // - Stage Mask VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT: It means "at the end of the subpass' pipeline".
        //
        // Finally, there is another level (beyond stage) where we can specify when operation needs to start/finish,
        // that's the Access Mask, basically means before/after "what operation within the stage".
        // The following website lists all the Access Mask values allowed and in what stages they can be used:
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html
        //
        std::array<VkSubpassDependency, 2> subpassDependencies;

        // Layout Conversion 1: VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        // Start after: End of whatever came before
        // Finish before: Color Output stage in subpass 0
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; // Conversion has to start after: it has to be read from
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Conversion has to finish before: reading or writing to it
        subpassDependencies[0].dependencyFlags = 0;

        // Layout Conversion 2: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        // Start after: Color Output stage in subpass 0
        // Finish before: Whatever comes after tries to read from it
        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Conversion has to start after: reading or writing to it
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; // Conversion has to finish before: it has to be read from
        subpassDependencies[1].dependencyFlags = 0;

        // -----------
        // Render Pass
        // 
        // This is the render pass we're building:
        //
        // RENDER PASS
        //      Color Attachment initial layout: VK_IMAGE_LAYOUT_UNDEFINED
        //
        //      Subpass dependency 1: Convert VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        // 
        //      SUBPASS 1
        //          Color Attachment layout: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        //          Draws to color attachment
        //
        //      Subpass dependency 2: Convert VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        //
        //      Color Attachment final layout: VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        //      Present color attachment to surface
        //
        VkRenderPassCreateInfo vkRenderPassCreateInfo = {};
        vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        vkRenderPassCreateInfo.pNext = nullptr;
        vkRenderPassCreateInfo.flags = 0;
        vkRenderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        vkRenderPassCreateInfo.pAttachments = attachments.data();
        vkRenderPassCreateInfo.subpassCount = 1;
        vkRenderPassCreateInfo.pSubpasses = &subpass;
        vkRenderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        vkRenderPassCreateInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(m_device->GetVkDevice(), &vkRenderPassCreateInfo, nullptr, &m_vkRenderPass) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan RenderPass", "Failed to create Vulkan RenderPass.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
