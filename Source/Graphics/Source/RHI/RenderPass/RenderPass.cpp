#include <RHI/RenderPass/RenderPass.h>

#include <RHI/Device/Device.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    RenderPass::RenderPass(Device* device, const RenderPassDesc& desc)
        : m_device(device)
        , m_desc(desc)
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
        if (m_desc.m_attachments.size() != 3)
        {
            DX_LOG(Fatal, "Vulkan RenderPass", "Expected 3 attachments.");
            return false;
        }

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
            // 0) SwapChain Image Attachment of Subpass 1
            {
                .flags = 0,
                .format = ToVkFormat(m_desc.m_attachments[0]), // Format to use for attachment
                .samples = VK_SAMPLE_COUNT_1_BIT, // Number of samples to write for MSAA

                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // What to do with attachment before render pass starts
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // What to do with attachment after render pass finishes
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // Image data layout expected before render pass starts
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // Image data layout to convert to after render pass finishes
            },
            // 1) Color Attachment of Subpass 0 and Input Attachment to Subpass 1
            {
                .flags = 0,
                .format = ToVkFormat(m_desc.m_attachments[1]), // Format to use for attachment
                .samples = VK_SAMPLE_COUNT_1_BIT, // Number of samples to write for MSAA

                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // What to do with attachment before render pass starts
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // What to do with attachment after render pass finishes
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // Image data layout expected before render pass starts
                .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // Image data layout to convert to after render pass finishes
            },
            // 2) Depth/Stencil Attachment of Subpass 0
            {
                .flags = 0,
                .format = ToVkFormat(m_desc.m_attachments[2]),
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
        // The implicit transition 1) will happen before the subpass starts and before the clear operation (which happens before the attachment is used within the pipeline)
        // The implicit transition 2) will happen after it's written by the subpass' pipeline and before the store operation.
        // 
        // IMPORTANT ==> But it will NOT do layout transition between subpasses!! If 2 subpasses specify different layouts for the same
        // attachment, then a subpass dependency is necessary.
        //
        // NOTE: A subpass doesn't have to use all attachments defined in the render pass.
        //       The render pass defines them all, the subpasses indicates which ones are used.
        const std::vector<VkAttachmentReference> colorAttachmentReferences = {
            {
                .attachment = 1, // Index of the attachment inside vkRenderPassCreateInfo.pAttachments
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // Image data layout to convert to before render subpass starts
            }
        };
        const VkAttachmentReference depthStencilAttachmentReference = {
            .attachment = 2,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
        const std::vector<VkAttachmentReference> swapChainAttachmentReferences = {
            {
                .attachment = 0, // Index of the attachment inside vkRenderPassCreateInfo.pAttachments
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // Image data layout to convert to before render subpass starts
            }
        };
        const std::vector<VkAttachmentReference> inputAttachmentReferences = {
            {
                .attachment = 1, // Index of the attachment inside vkRenderPassCreateInfo.pAttachments
                .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  // Image data layout to convert to before render subpass starts
            },
            {
                .attachment = 2, // Index of the attachment inside vkRenderPassCreateInfo.pAttachments
                .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  // Image data layout to convert to before render subpass starts
            }
        };

        const std::vector<VkSubpassDescription> subpasses = {
            // Subpass 0
            {
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
            },
            // Subpass 1
            {
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, // What pipeline the subpass is to be bound to
                .inputAttachmentCount = static_cast<uint32_t>(inputAttachmentReferences.size()),
                .pInputAttachments = inputAttachmentReferences.data(),
                .colorAttachmentCount = static_cast<uint32_t>(swapChainAttachmentReferences.size()),
                .pColorAttachments = swapChainAttachmentReferences.data(),
                .pResolveAttachments = 0,
                .pDepthStencilAttachment = nullptr,
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr
            }
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
        std::array<VkSubpassDependency, 1> subpassDependencies;

        // -------------------------------
        // Situation BEFORE when we had one subpass, which used the swap-chain image in subpass 0:
        // 
        // We needed a subpass dependency because the implicit layout transition from VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        // happens before the subpass starts, so it happened before the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage that the vkQueueSubmit will
        // wait for to check the swap chain's semaphore. Due to that semaphore wait, the clear and write operations
        // were safe, but not the implicit layout transition, which would happen before and therefore the swap-chain image
        // might still being presented. To made this safe we added a dependency between External subpass and subpass 0 so:
        // - Transition starts after external subpass (swap-chain) has finished reading from it.
        // - Transition finishes before subpass 0 trying to read/write from it at VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage.
        // 
        // subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        // subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;  // Conversion has to start after: swap chain has read from it
        // subpassDependencies[0].dstSubpass = 0;
        // subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Conversion has to finish before: reading or writing to color attachment
        // subpassDependencies[0].dependencyFlags = 0;
        // 
        // -------------------------------
        // Situation NOW with multiple subpasses is different, the swap-chain image is used in subpass 1, not 0:
        // 
        // The queue submit (vkQueueSubmit) will sync with the first VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage of the render pass,
        // which will be found by the first subpass (subpass 0). So the swap-chain image will be ready before any operation (implicit layout
        // transition or clear operation) is done in subpass 1. This means we don't need a subpass dependency now.
        // 
        // -------------------------------
        // The transition from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR is the same now.
        // It is handled implicitly and there is not issue with it, the swap-chain present is waiting for the semaphore that queue submit
        // has finished execution everything, that includes writing to the swap-chain image AND do the transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
        //
        // -------------------------------
        // Since subpass 1 uses as inputs the attachments from subpass 0, Vulkan guarantees that subpass 0 will be executed before subpass 1 starts.
        // So no dependency is needed with regard to execution order.
        // 
        // But the change of layout that happens to the color/depth attachments from subpass 0 to subpass 1 is NOT implicitly handled by Vulkan and
        // therefore we do need a subpass dependency for this:
        // 
        // Layout in subpass 0 (color/depth attachment) -> Layout in subpass 1 (shader read)
        // Start after: Color Output stage in subpass 0
        // Finish before: Fragment shader stage in subpass 1
        subpassDependencies[0].srcSubpass = 0;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Conversion has to start after: writing to it
        subpassDependencies[0].dstSubpass = 1;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; // Conversion has to finish before: it has to be read from
        subpassDependencies[0].dependencyFlags = 0;

        // -----------
        // Render Pass
        // 
        // This is the render pass we're building:
        //
        // RENDER PASS
        // 
        //      SUBPASS 0
        //          Color Attachment initial layout transition: VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        //          Depth Attachment initial layout transition: VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        //          Draws to color/depth attachment
        // 
        //      SUBPASS 1
        //          Color Input layout transition (subpass dependency 0): VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        //          Depth Input layout transition (subpass dependency 0): VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        //          Swap-Chain Image Attachment initial layout transition: VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        //          Draws to swap-chain image attachment
        //
        //      Color Attachment final layout conversion: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        //      Depth Attachment final layout conversion: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL -> VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        //      Swap Chain Image Attachment final layout conversion: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        //
        VkRenderPassCreateInfo vkRenderPassCreateInfo = {};
        vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        vkRenderPassCreateInfo.pNext = nullptr;
        vkRenderPassCreateInfo.flags = 0;
        vkRenderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        vkRenderPassCreateInfo.pAttachments = attachments.data();
        vkRenderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
        vkRenderPassCreateInfo.pSubpasses = subpasses.data();
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
