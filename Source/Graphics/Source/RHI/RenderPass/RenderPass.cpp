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
        // It's our responsibility to specify the correct image layouts for the images while they
        // are being used by render passes and subpasses.
        //
        // Special keywords to be aware of:
        // - Layout VK_IMAGE_LAYOUT_UNDEFINED: It means "we don't care what previous layout the image was in".

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
        // A subpass also specifies the layout expected for the attachment. There are 2 implicit layout transitions that
        // happen automatically:
        // 1) Between the render pass initial layout to the fist subpass layout 
        // 2) Between the last subpass layout to render pass final layout.
        // 
        // The implicit transition 1) will happen before the clear operation (which happens before it's used within the pipeline)
        // The implicit transition 2) will happen after it's written by the pipeline and before the store operation.
        // 
        // But it will NOT do layout transition between subpasses!! If 2 subpasses specify different layouts for the same
        // attachment, then a subpass dependency is necessary.
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
        // Vulkan guarantees subpass execution order if they have attachment dependencies (if one subpass writes to an attachment 
        // and another has it as input), but if subpasses are independent (don't share attachments or resources) then they might
        // execute in parallel.
        // 
        // Also, as indicated before, layout transitions between subpasses are not implicitly handled, so subpass dependencies are required.
        // 
        // In summary, these are some reasons to use subpass dependencies:
        // - To specify layout transitions between subpasses.
        // - Explicitly synchronize subpasses when necessary.
        // - Having a finer control to specify the points when the layout transitions need to happen.
        // 
        // With a subpass dependency we specify 2 points within subpasses:
        // - Source: the point within the first subpass (dependency) after work can start.
        // - Destination: the point within the second subpass (dependent) when work needs to be finished.
        // 
        // Notice we don't say explicitly when the work needs to happen, but indicated a range in time
        // when the GPU will need to do the work.
        //
        // In the subpass dependency we specify not only between which subpasses the operation need
        // to happen, but we also specify at what stage inside the subpass' pipeline can the operation start
        // and expected to finish. For example, start after Vertex Shader of subpass A and finish before
        // Fragment Shader of subpass C.
        //
        // Special keywords to be aware of:
        // - Subpass index VK_SUBPASS_EXTERNAL: It means "anything that takes place outside our subpasses".
        // - Stage Mask VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT: It means "at the beginning of the subpass' pipeline".
        // - Stage Mask VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT: It means "at the end of the subpass' pipeline".
        //
        // Finally, there is another level (beyond stage) where we can specify when operation needs to start/finish,
        // that's the Access Mask, basically means before/after "what operation within the stage".
        // The following website lists all the Access Mask values allowed and in what stages they can be used:
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html
        //
        std::array<VkSubpassDependency, 2> subpassDependencies;

        // In our case we need on dependency because the implicit layout transition from
        // VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL happens before the subpass starts,
        // so it happens before the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage that the vkQueueSubmit will
        // wait for to check the swap chain's semaphore. Due to that semaphore wait, the clear and write operations
        // are safe, but not the implicit layout transition, which will happen before and therefore the swapchain image
        // might still being presented. To made this safe we're going to add a dependency between External subpass and
        // subpass 0 so:
        // - Transition starts after external subpass (swapchain) has finished reading from it.
        // - Transition finishes before subpass 0 trying to read/write from it at VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage.
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;  // Conversion has to start after: swap chain has read from it
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Conversion has to finish before: reading or writing to color attachment
        subpassDependencies[0].dependencyFlags = 0;

        // Udemy course indicated other dependencies. But the transition from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        // is handled implicitly and there is not issue with it, the swap-chain present is waiting for the semaphore that queue submit
        // has finished execution everything, that includes writing to the swap-chain image AND do the transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.

        /*
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
        */

        // Vulkan tutorial also indicates that subpass dependency is required for the depth attachment, but that doesn't seem
        // to be the case because it's not used outside the render pass and the layout transition and clear operation will be
        // implicitly handled.

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
        //      Color Attachment final layout conversion: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
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
