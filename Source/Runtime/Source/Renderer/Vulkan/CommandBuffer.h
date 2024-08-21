#pragma once

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
        bool Begin(); // Call this first before the command calls.
        void End();   // Call this last after all the command calls.

        // -- Graphics commands --

        void BeginRenderPass(FrameBuffer* frameBuffer,
            std::optional<Math::Color> clearColor,
            std::optional<float> clearDepth = std::nullopt,
            std::optional<uint8_t> clearStencil = std::nullopt);
        void EndRenderPass();

        void BindPipeline(Pipeline* pipeline);

        void BindVertexBuffers(const std::vector<Buffer*>& vertexBuffers);
        void BindIndexBuffer(Buffer* indexBuffer);

        void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0,
            uint32_t instanceCount = 1, uint32_t firstInstance = 0);

        // -- Transfer commands --

        void CopyBuffer();

    private:
        Device* m_device = nullptr;
        VkCommandPool m_vkCommandPool = nullptr;

    private:
        bool AllocateVkCommandBuffer();

        VkCommandBuffer m_vkCommandBuffer = nullptr;
    };
} // namespace Vulkan
