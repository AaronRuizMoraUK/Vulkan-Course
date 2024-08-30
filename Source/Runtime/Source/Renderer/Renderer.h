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
    class Pipeline;
    class Buffer;
    class PipelineDescriptorSet;
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

        // WorldBuffers for all objects aligned to required offset for dynamic uniform buffers
        class AlignedWorldBuffers
        {
        public:
            static size_t SizeAlignedForDynamicUniformBuffer(Vulkan::Device* device);

            AlignedWorldBuffers(Vulkan::Device* device, size_t worldBufferCount);
            ~AlignedWorldBuffers();

            WorldBuffer* GetWorldBuffer(size_t index);
            size_t GetWorldBufferAlignedSize() const;

            const uint8_t* GetData() const;

        private:
            void AllocateData();
            void FreeData();

            Vulkan::Device* m_device = nullptr;
            size_t m_worldBufferCount = 0;
            size_t m_worldBufferAlignedSize = 0;
            uint8_t* m_data = nullptr;
        };

        void RecordCommands(uint32_t swapChainImageIndex);

    private:
        bool CreateInstance();
        bool CreateDevice();
        bool CreateSwapChain();

        std::unique_ptr<Vulkan::Instance> m_instance;
        std::unique_ptr<Vulkan::Device> m_device;
        std::unique_ptr<Vulkan::SwapChain> m_swapChain;

    private:
        bool CreatePipeline();
        bool CreateFrameBuffers();

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
        // NOTE: Since we are pre-recording all the commands and not every frame,
        //       we need as many elements as frame buffers (that's as many elements
        //       as swap chain images). Once the commands are generated every frame
        //       then it should be enough with having MaxFrameDraws elements instead.
        // 
        // TODO: Move out of renderer to a new class.
        bool CreateFrameData();

        // We need uniform buffers for each frame so they won't stumble into each other
        // while drawing the independent frames. They might have different content per frame.
        std::vector<std::shared_ptr<Vulkan::Buffer>> m_viewProjUniformBuffers;
        std::vector<std::shared_ptr<Vulkan::Buffer>> m_worldUniformBuffers;
        std::vector<std::unique_ptr<AlignedWorldBuffers>> m_alignedWorldBuffersData;
        // We need pipeline descriptor sets for each frame so they won't stumble into each other
        // while drawing the independent frames. They might have different content per frame.
        std::vector<std::shared_ptr<Vulkan::PipelineDescriptorSet>> m_perObjectDescritorSets;
    };
} // namespace DX
