#pragma once

#include <Window/Window.h>
#include <GenericId/GenericId.h>

#include <Math/Matrix4x4.h>

#include <vector>
#include <memory>
#include <unordered_set>

typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkFence_T* VkFence;

namespace Vulkan
{
    class Instance;
    class Device;
    class SwapChain;
    class RenderPass;
    class FrameBuffer;
    class Pipeline;
    class Buffer;
    class PipelineDescriptorSet;
    class CommandBuffer;
}

namespace DX
{
    class Camera;
    class Object;

    using RendererId = GenericId<struct RendererIdTag>;

    // Manages the render device, swap chain, frame buffer and scene.
    class Renderer
    {
    public:
        Renderer(RendererId rendererId, Window* window);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        bool Initialize();
        void Terminate();

        RendererId GetId() const { return m_rendererId; }

        Window* GetWindow();
        Vulkan::Device* GetDevice();

        void Render();

        // Wait until no actions being run on device before destroying.
        void WaitUntilIdle();

        void SetCamera(Camera* camera);

        void AddObject(Object* object);
        void RemoveObject(Object* object);

    private:
        void UpdateFrameData();
        void RecordCommands(Vulkan::FrameBuffer* frameBuffer);

        RendererId m_rendererId;
        Window* m_window = nullptr;

        // Camera
        Camera* m_camera = nullptr;

        // Scene objects
        std::unordered_set<Object*> m_objects;

        // Per Scene Resources
        struct ViewProjBuffer
        {
            Math::Matrix4x4Packed m_viewMatrix;
            Math::Matrix4x4Packed m_projMatrix;
            Math::Vector4Packed m_camPos;

            ViewProjBuffer(
                const Math::Matrix4x4& viewMatrix,
                const Math::Matrix4x4& projMatrix,
                const Math::Vector4& camPos)
                : m_viewMatrix(viewMatrix)
                , m_projMatrix(projMatrix)
                , m_camPos(camPos)
            {
                FlipYProj();
            }

        private:
            // Vulkan screen coordinates have negative Y values for the top of the screen.
            // This function will flip the Y coordinates to expected values.
            void FlipYProj()
            {
                m_projMatrix.columns[1].y *= -1.0f;
            }
        };

        // Per Object Resources
        struct WorldBuffer
        {
            Math::Matrix4x4Packed m_worldMatrix;
            Math::Matrix4x4Packed m_inverseTransposeWorldMatrix;
        };

    private:
        bool CreateInstance();
        bool CreateDevice();
        bool CreateSwapChain();

        std::unique_ptr<Vulkan::Instance> m_instance;
        std::unique_ptr<Vulkan::Device> m_device;
        std::unique_ptr<Vulkan::SwapChain> m_swapChain;

    private:
        bool CreateRenderPass();
        bool CreateFrameBuffers();
        bool CreatePipeline();

        std::unique_ptr<Vulkan::RenderPass> m_renderPass;
        std::unique_ptr<Vulkan::Pipeline> m_pipeline;

    private:
        // ---------------------------
        // Synchronization
        // 
        // TODO: Move out of renderer to a new class.
        bool CreateSynchronisation();

        int m_currentFrame = 0;

        // Used to know when the swap chain image is ready for drawing.
        std::vector<VkSemaphore> m_vkImageAvailableSemaphores;
        // Used to know when the execution of the command buffer in the
        // queue (rendering) has finished and therefore can be presented
        // in the swap chain image.
        std::vector<VkSemaphore> m_vkRenderFinishedSemaphores;
        // Used to know when a render frame hasn't finished and wait until it does.
        // It protected render function from doing more than MaxFrameDraws renders.
        std::vector<VkFence> m_vkRenderFences;

    private:
        // ---------------------------
        // Per Frame data
        // 
        // We need data for each frame so they won't stumble into each other
        // while drawing the independent frames. They might have different content per frame.
        // 
        // TODO: Move out of renderer to a new class.
        bool CreateFrameData();

        // Command buffers for sending commands to each swap chain frame buffer.
        std::vector<std::unique_ptr<Vulkan::CommandBuffer>> m_commandBuffers; // One per frame

        // Per Scene resources
        std::vector<std::unique_ptr<Vulkan::Buffer>> m_viewProjUniformBuffers; // One per frame
        std::vector<std::shared_ptr<Vulkan::PipelineDescriptorSet>> m_perSceneDescritorSets; // One per frame

        // Per Object resources
        using PipelineDescriptorSetsForObjects = std::vector<std::shared_ptr<Vulkan::PipelineDescriptorSet>>; // MaxObjects elements
        std::vector<PipelineDescriptorSetsForObjects> m_perObjectDescritorSets; // One per frame
    };
} // namespace DX
