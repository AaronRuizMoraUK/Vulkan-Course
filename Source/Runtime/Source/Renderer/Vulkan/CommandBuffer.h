#pragma once

#include <Renderer/Vulkan/CommandBufferEnums.h>
#include <Renderer/Vulkan/ShaderEnums.h>

#include <Math/Color.h>

#include <optional>
#include <vector>

typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkCommandPool_T* VkCommandPool;
typedef struct VkBuffer_T* VkBuffer;

namespace Vulkan
{
    class Device;
    class FrameBuffer;
    class Pipeline;
    class PipelineDescriptorSet;
    class Buffer;

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
        void Reset(); // Call this to reset command buffer outside Begin/End scope.
        bool Begin(CommandBufferUsageFlags flags = 0); // Call this first before the command calls.
        void End();   // Call this last after all the command calls.

        // -- Graphics commands --

        void BeginRenderPass(FrameBuffer* frameBuffer,
            std::optional<Math::Color> clearColor,
            std::optional<float> clearDepth = std::nullopt,
            std::optional<uint8_t> clearStencil = std::nullopt);
        void EndRenderPass();

        void BindPipeline(Pipeline* pipeline);

        void BindPipelineDescriptorSet(PipelineDescriptorSet* descriptorSet);
        void BindPipelineDescriptorSet(PipelineDescriptorSet* descriptorSet, const std::vector<uint32_t>& dynamicOffsetsInBytes);
        void PushConstantsToPipeline(Pipeline* pipeline, ShaderType shaderType, const void* data, uint32_t dataSize, uint32_t offset = 0);

        void BindVertexBuffers(const std::vector<Buffer*>& vertexBuffers);
        void BindIndexBuffer(Buffer* indexBuffer);

        void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0,
            uint32_t instanceCount = 1, uint32_t firstInstance = 0);

        // -- Transfer commands --

        void CopyBuffer(VkBuffer vkDstBuffer, VkBuffer vkSrcBuffer, size_t bufferSize);

    private:
        Device* m_device = nullptr;
        VkCommandPool m_vkCommandPool = nullptr;

    private:
        bool AllocateVkCommandBuffer();

        VkCommandBuffer m_vkCommandBuffer = nullptr;
    };
} // namespace Vulkan
