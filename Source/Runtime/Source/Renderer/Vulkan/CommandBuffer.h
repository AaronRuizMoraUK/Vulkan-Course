#pragma once

#include <Math/Color.h>
#include <optional>

typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkCommandPool_T* VkCommandPool;

namespace Vulkan
{
    class Device;
    class FrameBuffer;
    class Pipeline;

    // Manages a Vulkan Command Buffer
    class CommandBuffer
    {
    public:
        CommandBuffer(Device* device, VkCommandPool vkCommandPool);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        bool Initialize(bool createDepthAttachment = false);
        void Terminate();

        // -----------------------------------------------------------------------------
        // Call the following functions asynchronously from a thread to record commands.
        // -----------------------------------------------------------------------------
        bool Begin(); // Call this first before the command calls.
        void End();   // Call this last after all the command calls.

        void BeginRenderPass(FrameBuffer* frameBuffer,
            std::optional<Math::Color> clearColor,
            std::optional<float> clearDepth = std::nullopt,
            std::optional<uint8_t> clearStencil = std::nullopt);
        void EndRenderPass();

        void BindPipeline(Pipeline* pipeline);

        void Draw(uint32_t vertexCount, uint32_t firstVertex = 0, 
            uint32_t instanceCount = 1, uint32_t firstInstance = 0);

    private:
        Device* m_device = nullptr;
        VkCommandPool m_vkCommandPool = nullptr;

    private:
        bool AllocateVkCommandBuffer();

        VkCommandBuffer m_vkCommandBuffer = nullptr;
    };
} // namespace Vulkan
