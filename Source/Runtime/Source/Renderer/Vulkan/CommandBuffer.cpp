#include <Renderer/Vulkan/CommandBuffer.h>

#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/FrameBuffer.h>
#include <Renderer/Vulkan/Pipeline.h>
#include <Renderer/Vulkan/PipelineDescriptorSet.h>
#include <Renderer/Vulkan/Buffer.h>
#include <Renderer/Vulkan/Utils.h>

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

        DX_LOG(Info, "Vulkan CommandBuffer", "Initializing Vulkan CommandBuffer...");

        if (!AllocateVkCommandBuffer())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void CommandBuffer::Terminate()
    {
        DX_LOG(Info, "Vulkan CommandBuffer", "Terminating Vulkan CommandBuffer...");

        vkFreeCommandBuffers(m_device->GetVkDevice(), m_vkCommandPool, 1, &m_vkCommandBuffer);
        m_vkCommandBuffer = nullptr;
    }

    VkCommandBuffer CommandBuffer::GetVkCommandBuffer()
    {
        return m_vkCommandBuffer;
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
        std::optional<Math::Color> clearColor,
        std::optional<float> clearDepth,
        std::optional<uint8_t> clearStencil)
    {
        const auto& imageSize = frameBuffer->GetColorImage().m_size;

        // Clear values needs to match 1:1 with attachments in frame buffer
        std::vector<VkClearValue> clearValues;
        if (clearColor.has_value())
        {
            const VkClearValue colorClearValue = {
                .color = { clearColor->x, clearColor->y, clearColor->z, clearColor->w }
            };
            clearValues.push_back(colorClearValue);
        }

        if (clearDepth.has_value() || clearStencil.has_value())
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
        vkRenderPassBeginInfo.renderPass = frameBuffer->GetVkRenderPass();
        vkRenderPassBeginInfo.framebuffer = frameBuffer->GetVkFrameBuffer();
        vkRenderPassBeginInfo.renderArea.offset = { 0, 0 };
        vkRenderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(imageSize.x);
        vkRenderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(imageSize.y);
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
            descriptorSet->GetVkPipelineLayout(),
            0, // Index of first descriptor set to bind (TODO: to obtain from PipelineDescriptorSet)
            static_cast<uint32_t>(vkDescriptorSets.size()),
            vkDescriptorSets.data(),
            0, // Dynamic offset count
            nullptr);
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

    void CommandBuffer::CopyBuffer(VkBuffer vkDstBuffer, VkBuffer vkSrcBuffer, size_t bufferSize)
    {
        // Region of data to copy from and to
        const VkBufferCopy vkBufferCopyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = bufferSize
        };

        vkCmdCopyBuffer(m_vkCommandBuffer, vkSrcBuffer, vkDstBuffer, 1, &vkBufferCopyRegion);
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
