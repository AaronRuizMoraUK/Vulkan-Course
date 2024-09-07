#pragma once

#include <RHI/CommandBuffer/CommandBufferEnums.h>
#include <RHI/Shader/ShaderEnums.h>

#include <Math/Color.h>

#include <optional>
#include <vector>

typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkCommandPool_T* VkCommandPool;

namespace Vulkan
{
    class Device;
    class FrameBuffer;
    class Pipeline;
    class PipelineDescriptorSet;
    class Buffer;
    class Image;

    // Manages a Vulkan Command Buffer
    class CommandBuffer
    {
    public:
        CommandBuffer(Device* device, VkCommandPool vkCommandPool);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        bool Initialize();
        void Terminate();

        VkCommandBuffer GetVkCommandBuffer();

        // -----------------------------------------------------------------------------
        // These functions can be called asynchronously from a thread to record commands.
        // -----------------------------------------------------------------------------
        bool Begin(CommandBufferUsageFlags flags = 0); // Call this first before the command calls.
        void End();   // Call this last after all the command calls.

        // Call this to reset command buffer outside Begin/End scope.
        // Make sure GPU is not using the command buffer before resetting it.
        // Resetting doesn't necessarily free memory from the pool, just sets 
        // the command buffer to initial state so it can be reused. The pool used to create
        // this command buffer needs the flag VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT.
        void Reset();

        // -- Graphics commands --

        // Begin a render pass to an specific frame buffer.
        // The render pass is obtained from the frame buffer as it stores which render pass is compatible with.
        void BeginRenderPass(FrameBuffer* frameBuffer,
            std::optional<Math::Color> clearColor = std::nullopt,
            std::optional<float> clearDepth = std::nullopt,
            std::optional<uint8_t> clearStencil = std::nullopt);
        void EndRenderPass();

        void NextSubpass();

        // Sets which pipeline the render pass will use when calling draw.
        // The pipeline needs to compatible with the render pass.
        // 
        // One pipeline can only be used with one subpass of the render pass. This is how it works when a
        // render pass has multiple subpasses: Inside the Begin/EndRenderPass block it needs to bind and
        // draw using the pipeline for the first subpass, then we indicate to move to the next subpass and
        // bind and draw with the pipeline of second subpass... and so on for how many subpasses the render pass has.
        void BindPipeline(Pipeline* pipeline);

        void BindPipelineDescriptorSet(PipelineDescriptorSet* descriptorSet);
        void BindPipelineDescriptorSet(PipelineDescriptorSet* descriptorSet, const std::vector<uint32_t>& dynamicOffsetsInBytes);
        void PushConstantsToPipeline(Pipeline* pipeline, ShaderTypeFlags shaderTypes, const void* data, uint32_t dataSize, uint32_t offset = 0);

        void BindVertexBuffers(const std::vector<Buffer*>& vertexBuffers);
        void BindIndexBuffer(Buffer* indexBuffer);

        void Draw(uint32_t vertexCount, uint32_t firstVertex = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0);
        void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0);

        // -- Transfer commands --

        void CopyBuffer(Buffer* dstBuffer, Buffer* srcBuffer);
        void CopyBufferToImage(Image* dstImage, Buffer* srcBuffer);

        // -- Barrier commands --

        void PipelineImageMemoryBarrier(Image* image, 
            int oldImageLayout, int newImageLayout,
            int srcPipelineStage, int srcAccessMask,
            int dstPipelineStage, int dstAccessMask);

    private:
        Device* m_device = nullptr;
        VkCommandPool m_vkCommandPool = nullptr;

    private:
        bool AllocateVkCommandBuffer();

        VkCommandBuffer m_vkCommandBuffer = nullptr;
    };
} // namespace Vulkan
