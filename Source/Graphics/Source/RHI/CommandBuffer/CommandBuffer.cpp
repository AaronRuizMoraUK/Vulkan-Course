#include <RHI/CommandBuffer/CommandBuffer.h>

#include <RHI/Device/Device.h>
#include <RHI/RenderPass/RenderPass.h>
#include <RHI/FrameBuffer/FrameBuffer.h>
#include <RHI/Pipeline/Pipeline.h>
#include <RHI/Pipeline/PipelineDescriptorSet.h>
#include <RHI/Resource/Buffer/Buffer.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    CommandBuffer::CommandBuffer(Device* device, VkCommandPool vkCommandPool)
        : m_device(device)
        , m_vkCommandPool(vkCommandPool)
    {
    }

    CommandBuffer::~CommandBuffer()
    {
        Terminate();
    }

    bool CommandBuffer::Initialize()
    {
        if (m_vkCommandBuffer)
        {
            return true; // Already initialized
        }

        //DX_LOG(Info, "Vulkan CommandBuffer", "Initializing Vulkan CommandBuffer...");

        if (!AllocateVkCommandBuffer())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void CommandBuffer::Terminate()
    {
        //DX_LOG(Info, "Vulkan CommandBuffer", "Terminating Vulkan CommandBuffer...");

        vkFreeCommandBuffers(m_device->GetVkDevice(), m_vkCommandPool, 1, &m_vkCommandBuffer);
        m_vkCommandBuffer = nullptr;
    }

    VkCommandBuffer CommandBuffer::GetVkCommandBuffer()
    {
        return m_vkCommandBuffer;
    }

    void CommandBuffer::Reset()
    {
        vkResetCommandBuffer(m_vkCommandBuffer, 0/*VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT*/);
    }

    bool CommandBuffer::Begin(CommandBufferUsageFlags flags)
    {
        VkCommandBufferBeginInfo bufferBeginInfo = {};
        bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bufferBeginInfo.pNext = nullptr;
        bufferBeginInfo.flags = ToVkCommandBufferUsageFlags(flags);
        bufferBeginInfo.pInheritanceInfo = nullptr;

        // Start recording commands to command buffer!
        if (vkBeginCommandBuffer(m_vkCommandBuffer, &bufferBeginInfo) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan CommandBuffer", "Failed to begin Vulkan CommandBuffer.");
            return false;
        }

        return true;
    }

    void CommandBuffer::End()
    {
        // End recording commands to command buffer!
        if (vkEndCommandBuffer(m_vkCommandBuffer) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan CommandBuffer", "Failed to end Vulkan CommandBuffer.");
        }
    }

    void CommandBuffer::BeginRenderPass(
        FrameBuffer* frameBuffer,
        std::vector<Math::Color> clearColors,
        std::optional<float> clearDepth,
        std::optional<uint8_t> clearStencil)
    {
        const size_t numColorAttachments = frameBuffer->GetFrameBufferDesc().m_colorAttachments.size();
        const bool hasDepthStencilAttachment = frameBuffer->GetFrameBufferDesc().m_depthStencilAttachment.m_image != nullptr;
        const Math::Vector2Int& frameBufferDimensions = frameBuffer->GetDimensions();

        if (numColorAttachments != clearColors.size())
        {
            DX_LOG(Warning, "Vulkan CommandBuffer", 
                "Frame buffer has %d color attachments but %d clear color values were provided.",
                numColorAttachments, clearColors.size());
        }

        if (hasDepthStencilAttachment && !clearDepth.has_value() && !clearStencil.has_value())
        {
            DX_LOG(Warning, "Vulkan CommandBuffer",
                "Frame buffer has a depth stencil attachment but no clear values for depth or stencil were provided.");
        }

        // Clear values needs to match 1:1 with attachments in frame buffer

        std::vector<VkClearValue> clearValues;
        for (size_t i = 0; i < numColorAttachments; ++i)
        {
            const VkClearValue colorClearValue = {
                .color = (i < clearColors.size())
                    ? VkClearColorValue{ clearColors[i].x, clearColors[i].y, clearColors[i].z, clearColors[i].w }
                    : VkClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f }
            };
            clearValues.push_back(colorClearValue);
        }

        if (hasDepthStencilAttachment)
        {
            const VkClearValue depthStencilClearValue = {
                .depthStencil = {
                    .depth = clearDepth.value_or(1.0f),
                    .stencil = clearStencil.value_or(0)
                }
            };
            clearValues.push_back(depthStencilClearValue);
        }

        // Information about how to begin a render pass (only needed for graphics operations)
        VkRenderPassBeginInfo vkRenderPassBeginInfo = {};
        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = nullptr;
        vkRenderPassBeginInfo.renderPass = frameBuffer->GetFrameBufferDesc().m_renderPass->GetVkRenderPass();
        vkRenderPassBeginInfo.framebuffer = frameBuffer->GetVkFrameBuffer();
        vkRenderPassBeginInfo.renderArea.offset = { 0, 0 };
        vkRenderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(frameBufferDimensions.x);
        vkRenderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(frameBufferDimensions.y);
        vkRenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        vkRenderPassBeginInfo.pClearValues = clearValues.data();

        // All the commands are going to be inline draws (no secondary level command buffers)
        const VkSubpassContents vkSubpassContents = VK_SUBPASS_CONTENTS_INLINE;

        vkCmdBeginRenderPass(m_vkCommandBuffer, &vkRenderPassBeginInfo, vkSubpassContents);
    }

    void CommandBuffer::EndRenderPass()
    {
        vkCmdEndRenderPass(m_vkCommandBuffer);
    }

    void CommandBuffer::BindPipeline(Pipeline* pipeline)
    {
        vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline());
    }

    void CommandBuffer::BindPipelineDescriptorSet(PipelineDescriptorSet* descriptorSet)
    {
        const std::vector<VkDescriptorSet> vkDescriptorSets = {
            descriptorSet->GetVkDescriptorSet()
        };

        vkCmdBindDescriptorSets(m_vkCommandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            descriptorSet->GetPipeline()->GetVkPipelineLayout(),
            descriptorSet->GetSetLayoutIndex(), // Index of the descriptor set inside the pipeline layout
            static_cast<uint32_t>(vkDescriptorSets.size()),
            vkDescriptorSets.data(),
            0, // Dynamic offset count
            nullptr);
    }

    void CommandBuffer::BindPipelineDescriptorSet(
        PipelineDescriptorSet* descriptorSet, 
        const std::vector<uint32_t>& dynamicOffsetsInBytes)
    {
        if (dynamicOffsetsInBytes.size() != descriptorSet->GetDescriptorSetLayout()->m_numDynamicDescriptors)
        {
            DX_LOG(Error, "CommandBuffer", 
                "Number of dynamic descriptors in set layout (%d) does not match the number of dynamic offsets passed (%d).",
                descriptorSet->GetDescriptorSetLayout()->m_numDynamicDescriptors, dynamicOffsetsInBytes.size());
            return;
        }

        const std::vector<VkDescriptorSet> vkDescriptorSets = {
            descriptorSet->GetVkDescriptorSet()
        };

        vkCmdBindDescriptorSets(m_vkCommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            descriptorSet->GetPipeline()->GetVkPipelineLayout(),
            descriptorSet->GetSetLayoutIndex(), // Index of the descriptor set inside the pipeline layout
            static_cast<uint32_t>(vkDescriptorSets.size()),
            vkDescriptorSets.data(),
            static_cast<uint32_t>(dynamicOffsetsInBytes.size()), // Dynamic offset count
            dynamicOffsetsInBytes.data());
    }

    void CommandBuffer::PushConstantsToPipeline(
        Pipeline* pipeline, 
        ShaderTypeFlags shaderTypes,
        const void* data, 
        uint32_t dataSize, 
        uint32_t offset)
    {
        // Max size is 128 bytes
        if (dataSize > PushConstantsMaxSize)
        {
            DX_LOG(Error, "CommandBuffer",
                "Pushing %d bytes of data into the pipeline, which is greater than the max size allowed of %d bytes.",
                dataSize, PushConstantsMaxSize);
            return;
        }

        vkCmdPushConstants(m_vkCommandBuffer, 
            pipeline->GetVkPipelineLayout(), 
            ToVkShaderStageFlags(shaderTypes),
            offset,
            dataSize,
            data);
    }

    void CommandBuffer::BindVertexBuffers(const std::vector<Buffer*>& vertexBuffers)
    {
        std::vector<VkBuffer> vkVertexBuffers(vertexBuffers.size());
        std::vector<VkDeviceSize> vkDeviceSizes(vertexBuffers.size());

        for (int i = 0; i < vertexBuffers.size(); ++i)
        {
            vkVertexBuffers[i] = vertexBuffers[i]->GetVkBuffer();
            vkDeviceSizes[i] = 0; // Offset into buffer being bound
        }

        vkCmdBindVertexBuffers(m_vkCommandBuffer, 
            0, static_cast<uint32_t>(vkVertexBuffers.size()), vkVertexBuffers.data(), vkDeviceSizes.data());
    }

    void CommandBuffer::BindIndexBuffer(Buffer* indexBuffer)
    {
        VkIndexType vkIndexType;
        switch (indexBuffer->GetBufferDesc().m_elementSizeInBytes)
        {
        case 2:
            vkIndexType = VK_INDEX_TYPE_UINT16;
            break;
        case 4:
            vkIndexType = VK_INDEX_TYPE_UINT32;
            break;
        default:
            DX_LOG(Fatal, "CommandBuffer", "Index type not supported.");
            return;
        }

        vkCmdBindIndexBuffer(m_vkCommandBuffer, indexBuffer->GetVkBuffer(), 0, vkIndexType);
    }

    void CommandBuffer::DrawIndexed(
        uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset,
        uint32_t instanceCount, uint32_t firstInstance)
    {
        vkCmdDrawIndexed(m_vkCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void CommandBuffer::CopyBuffer(Buffer* dstBuffer, Buffer* srcBuffer)
    {
        DX_ASSERT(srcBuffer->GetBufferDesc().m_elementSizeInBytes == dstBuffer->GetBufferDesc().m_elementSizeInBytes,
            "Command Buffer", "Cannot copy buffers with different element size");
        DX_ASSERT(srcBuffer->GetBufferDesc().m_elementCount <= dstBuffer->GetBufferDesc().m_elementCount,
            "Command Buffer", "Trying to copy %d elements into a buffer with %d elements",
            srcBuffer->GetBufferDesc().m_elementCount, dstBuffer->GetBufferDesc().m_elementCount);

        // Region of data to copy from and to
        const VkBufferCopy vkBufferCopyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = srcBuffer->GetBufferDesc().m_elementSizeInBytes * srcBuffer->GetBufferDesc().m_elementCount
        };

        vkCmdCopyBuffer(m_vkCommandBuffer, srcBuffer->GetVkBuffer(), dstBuffer->GetVkBuffer(), 1, &vkBufferCopyRegion);
    }

    void CommandBuffer::CopyBufferToImage(Image* dstImage, Buffer* srcBuffer)
    {
        // Generate image regions (one per mip level) to copy
        std::vector<VkBufferImageCopy> vkBufferImageCopyRegions;
        {
            const ResourceFormat imageFormat = dstImage->GetImageDesc().m_format;
            const Math::Vector3Int& imageDimensions = dstImage->GetImageDesc().m_dimensions;
            const uint32_t imageMipCount = dstImage->GetImageDesc().m_mipCount;

            vkBufferImageCopyRegions.resize(imageMipCount);

            uint32_t bufferOffset = 0;
            for (uint32_t mipLevel = 0; mipLevel < imageMipCount; ++mipLevel)
            {
                const uint32_t mipSizeX = std::max<uint32_t>(1, imageDimensions.x >> mipLevel);
                const uint32_t mipSizeY = std::max<uint32_t>(1, imageDimensions.y >> mipLevel);
                const uint32_t mipSizeZ = std::max<uint32_t>(1, imageDimensions.z >> mipLevel);
                const uint32_t mipBytes = ResourceFormatSize(imageFormat, mipSizeX * mipSizeY * mipSizeZ);

                vkBufferImageCopyRegions[mipLevel] = VkBufferImageCopy{
                    .bufferOffset = bufferOffset,
                    .bufferRowLength = 0, // When row or image lengths are zero the memory is considered to be tightly packed according to the imageExtent
                    .bufferImageHeight = 0,
                    .imageSubresource = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // Which aspect of image to copy
                        .mipLevel = mipLevel, // Mip level to copy
                        .baseArrayLayer = 0, // Starting array layer (if array)
                        .layerCount = 1 // Number of layers to copy starting from baseArrayLayer
                    },
                    .imageOffset = {0, 0, 0},
                    .imageExtent = {mipSizeX, mipSizeY, mipSizeZ}
                };

                bufferOffset += mipBytes;
            }

            DX_ASSERT(srcBuffer->GetBufferDesc().m_elementSizeInBytes * srcBuffer->GetBufferDesc().m_elementCount <= bufferOffset,
                "Command Buffer", "Trying to copy %d bytes into a texture with %d bytes",
                srcBuffer->GetBufferDesc().m_elementSizeInBytes * srcBuffer->GetBufferDesc().m_elementCount, bufferOffset);
        }

        // How is the image memory set to be read and written to.
        const VkImageLayout vkDstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        vkCmdCopyBufferToImage(m_vkCommandBuffer, srcBuffer->GetVkBuffer(), dstImage->GetVkImage(), vkDstImageLayout, 
            static_cast<uint32_t>(vkBufferImageCopyRegions.size()), vkBufferImageCopyRegions.data());
    }

    void CommandBuffer::PipelineImageMemoryBarrier(Image* image, 
        int oldImageLayout, int newImageLayout,
        int srcPipelineStage, int srcAccessMask,
        int dstPipelineStage, int dstAccessMask)
    {
        const VkImageLayout vkOldImageLayout = static_cast<VkImageLayout>(oldImageLayout);
        const VkImageLayout vkNewImageLayout = static_cast<VkImageLayout>(newImageLayout);

        // About barriers
        // 
        // A barrier specifies dependencies between stages in a pipeline.
        // In other words, what stages of a pipeline depend on others finishing first.
        // Same as subpass dependencies it goes another level deeper and can specify
        // the Access Mask inside the stage, that's "what operation within the stage".
        //
        // On top of specifying dependencies, the barrier can also achieve 2 more things:
        // - Change the layout between the two stages specified.
        // - Change the queue family between the two stages specified.
        //
        // Out main goal in this function using the barrier is to change the image layout from
        // VK_IMAGE_LAYOUT_UNDEFINED (how it was created in CreateVkImage) to
        // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL (expected by CopyBufferToImage).
        //
        // Note that the stages are specified as parameters in vkCmdPipelineBarrier and their
        // access masks in VkImageMemoryBarrier.
        //
        // There are other types of barriers, this is a image memory barrier. There are also
        // global memory and buffer memory barriers, but all of them work in the similar way
        // as explained here.

        // Transition must happen AFTER srcPipelineStage stage and srcAccessMask access.
        // Transition must happen BEFORE dstPipelineStage stage and dstAccessMask access.

        VkImageMemoryBarrier vkImageMemoryBarrier = {};
        vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vkImageMemoryBarrier.pNext = nullptr;
        vkImageMemoryBarrier.srcAccessMask = srcAccessMask; // Memory operation (in src stage) indicating "after this point"
        vkImageMemoryBarrier.dstAccessMask = dstAccessMask; // Memory operation (in dst stage) indicating "before this point"
        vkImageMemoryBarrier.oldLayout = vkOldImageLayout; // Layout to transition from
        vkImageMemoryBarrier.newLayout = vkNewImageLayout; // Layout to transition too
        vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue family to transition from
        vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue family to transition to
        vkImageMemoryBarrier.image = image->GetVkImage();
        vkImageMemoryBarrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // Aspect of image being altered
            .baseMipLevel = 0, // First mip level to start alterations on
            .levelCount = image->GetImageDesc().m_mipCount, // Number of mip level to alter staring from baseMipLevel
            .baseArrayLayer = 0, // First array to start alterations on
            .layerCount = 1 // Number of arrays to alter staring from baseArrayLayer
        };

        vkCmdPipelineBarrier(m_vkCommandBuffer,
            srcPipelineStage, dstPipelineStage,
            0, // Dependency flags
            0, nullptr, // Global memory barriers
            0, nullptr, // Buffer memory barriers
            1, &vkImageMemoryBarrier); // Image memory barriers
    }

    bool CommandBuffer::AllocateVkCommandBuffer()
    {
        VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo = {};
        vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        vkCommandBufferAllocateInfo.pNext = nullptr;
        vkCommandBufferAllocateInfo.commandPool = m_vkCommandPool;
        vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Primary cmd buffers submit directly to queue, secondary to other cmd buffers.
        vkCommandBufferAllocateInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(
            m_device->GetVkDevice(), &vkCommandBufferAllocateInfo, &m_vkCommandBuffer) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan CommandBuffer", "Failed to create Vulkan CommandBuffer.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
