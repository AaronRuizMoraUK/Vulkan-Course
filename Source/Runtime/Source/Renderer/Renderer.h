#pragma once

#include <Window/Window.h>
#include <GenericId/GenericId.h>

#include <array>
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
}

namespace DX
{
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

        void AddObject(Object* object);
        void RemoveObject(Object* object);

        // TODO: Remove this function and inside Render() record the
        //       commands every frame the current frame command buffer.
        void RecordCommands();

    private:
        RendererId m_rendererId;
        Window* m_window = nullptr;

        std::unordered_set<Object*> m_objects;

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
        // TODO: Move out of renderer. Maybe to SwapChain class or a new class.
        bool CreateSynchronisation();

        // MaxFrameDraws needs to be lower than number of images in swap chain,
        // that way it'll block until there are images available for drawing and
        // won't affect the one being presented.
        static const int MaxFrameDraws = 2; 
        int m_currentFrame = 0;

        // Used to know when the swap chain image is ready for drawing.
        std::array<VkSemaphore, MaxFrameDraws> m_vkImageAvailableSemaphores;
        // Used to know when the execution of the command buffer in the
        // queue (rendering) has finished and therefore can be presented
        // in the swap chain image.
        std::array<VkSemaphore, MaxFrameDraws> m_vkRenderFinishedSemaphores;
        // Used to know when a render frame hasn't finished and wait until it does.
        // It protected render function from doing more than MaxFrameDraws renders.
        std::array<VkFence, MaxFrameDraws> m_vkRenderFences;
    };
} // namespace DX
