#include <Renderer/Renderer.h>

#include <Renderer/Object.h>
#include <Renderer/Vulkan/Instance.h>
#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/SwapChain.h>
#include <Renderer/Vulkan/Pipeline.h>
#include <Renderer/Vulkan/CommandBuffer.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace DX
{
    Renderer::Renderer(RendererId rendererId, Window* window)
        : m_rendererId(rendererId)
        , m_window(window)
    {
        m_vkImageAvailableSemaphores.fill(nullptr);
        m_vkRenderFinishedSemaphores.fill(nullptr);
        m_vkRenderFences.fill(nullptr);
    }

    Renderer::~Renderer()
    {
        Terminate();
    }

    bool Renderer::Initialize()
    {
        if (m_device)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Renderer", "Initializing Renderer...");

        if (!CreateInstance())
        {
            Terminate();
            return false;
        }

        if (!CreateDevice())
        {
            Terminate();
            return false;
        }

        if (!CreateSwapChain())
        {
            Terminate();
            return false;
        }

        if (!CreatePipeline())
        {
            Terminate();
            return false;
        }

        if (!CreateFrameBuffers())
        {
            Terminate();
            return false;
        }

        if (!CreateSynchronisation())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Renderer::Terminate()
    {
        // Necessary before destroying synchronization data
        WaitUntilIdle();

        DX_LOG(Info, "Renderer", "Terminating Renderer...");

        if (m_device)
        {
            for (int i = 0; i < MaxFrameDraws; ++i)
            {
                vkDestroySemaphore(m_device->GetVkDevice(), m_vkImageAvailableSemaphores[i], nullptr);
                vkDestroySemaphore(m_device->GetVkDevice(), m_vkRenderFinishedSemaphores[i], nullptr);
                vkDestroyFence(m_device->GetVkDevice(), m_vkRenderFences[i], nullptr);
            }
        }
        m_vkImageAvailableSemaphores.fill(nullptr);
        m_vkRenderFinishedSemaphores.fill(nullptr);
        m_vkRenderFences.fill(nullptr);

        m_pipeline.reset();
        m_swapChain.reset();
        m_device.reset();
        m_instance.reset();
    }

    Window* Renderer::GetWindow()
    {
        return m_window;
    }

    Vulkan::Device* Renderer::GetDevice()
    {
        return m_device.get();
    }

    void Renderer::Render()
    {
        // About Vulkan Semaphores
        // 
        // Mechanism to achieve GPU-GPU synchronization.
        // On CPU we create the semaphore and then we pass it
        // to a vulkan function so the GPU that will signal it later,
        // and we will also pass it to other vulkan function so the GPU
        // that will wait for it.

        // About Vulkan Fences
        // 
        // Mechanism to achieve GPU-CPU synchronization. We can wait in CPU
        // for a fence to signal (be opened) and on CPU we can also reset it (close it).
        // Then we can pass the fence to a vulkan function so GPU will signal it.

        constexpr uint64_t noTimeOut = std::numeric_limits<uint64_t>::max();

        // 0) Wait for current frame's render fence to signal (be opened) from last draw before continuing.
        vkWaitForFences(m_device->GetVkDevice(), 1, &m_vkRenderFences[m_currentFrame], VK_TRUE, noTimeOut);
        // Reset (close) the fence, it means it's in use, then "vkQueueSubmit" later will mark it as open when finished.
        vkResetFences(m_device->GetVkDevice(), 1, &m_vkRenderFences[m_currentFrame]);

        // 1) Get next available image to draw to and pass a semaphore so the GPU will signal
        //    when the image is available.
        //
        // NOTE: vkAcquireNextImageKHR call only blocks until it knows what the next
        //       image available will be. It will not block until that image is actually available,
        //       that's what the semaphore passed to it is for.
        uint32_t swapChainImageIndex = 0;
        if (vkAcquireNextImageKHR(m_device->GetVkDevice(), m_swapChain->GetVkSwapChain(),
            noTimeOut, m_vkImageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &swapChainImageIndex) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to acquire next image from swap chain.");
            return;
        }

        // 2) Submit the command buffer (of the current image) to the queue for execution.
        //    Wait at the convenient stage within the pipeline for the image semaphore to be signaled (so it's available for drawing to it).
        //    For example, allow to execute vertex shader, but wait for the image semaphore to be available before executing fragment shader.
        //    Lastly signal (with a different semaphore) when it has finished rendering.
        const std::vector<VkPipelineStageFlags> waitStages = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
        VkCommandBuffer currentCommandBuffer = m_swapChain->GetCommandBuffer(swapChainImageIndex)->GetVkCommandBuffer();

        VkSubmitInfo vkSubmitInfo = {};
        vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        vkSubmitInfo.pNext = nullptr;
        vkSubmitInfo.pWaitDstStageMask = waitStages.data(); // Stages to check semaphores at. Execute until this stage, then wait on semaphores to be ready.
        vkSubmitInfo.waitSemaphoreCount = 1; // Semaphores to wait on
        vkSubmitInfo.pWaitSemaphores = &m_vkImageAvailableSemaphores[m_currentFrame]; // Wait for GPU to signal m_vkImageAvailableSemaphore passed to vkAcquireNextImageKHR
        vkSubmitInfo.commandBufferCount = 1;
        vkSubmitInfo.pCommandBuffers = &currentCommandBuffer;
        vkSubmitInfo.signalSemaphoreCount = 1;
        vkSubmitInfo.pSignalSemaphores = &m_vkRenderFinishedSemaphores[m_currentFrame]; // Semaphore that will signal when this command buffer has finished executing

        // Submit command buffer to queue for execution by the GPU
        // Pass the render fence of the current frame, so when it's finished drawing it will signal it.
        if (vkQueueSubmit(m_device->GetVkQueue(Vulkan::QueueFamilyType_Graphics),
            1, &vkSubmitInfo, m_vkRenderFences[m_currentFrame]) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to submit work to the queue.");
            return;
        }
        
        // 3) Present image to screen when it has signaled that it has finished rendering.
        const VkSwapchainKHR vkSwapChain = m_swapChain->GetVkSwapChain();

        VkPresentInfoKHR vkPresentInfoKHR = {};
        vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        vkPresentInfoKHR.pNext = nullptr;
        vkPresentInfoKHR.waitSemaphoreCount = 1;
        vkPresentInfoKHR.pWaitSemaphores = &m_vkRenderFinishedSemaphores[m_currentFrame]; // Wait until this semaphore is signaled before presenting.
        vkPresentInfoKHR.swapchainCount = 1;
        vkPresentInfoKHR.pSwapchains = &vkSwapChain; // SwapChains to present the images to
        vkPresentInfoKHR.pImageIndices = &swapChainImageIndex; // Image indices in swap chains to present
        vkPresentInfoKHR.pResults = nullptr;

        if (vkQueuePresentKHR(m_device->GetVkQueue(Vulkan::QueueFamilyType_Presentation), 
            &vkPresentInfoKHR) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to present image.");
            return;
        }

        // Next frame
        m_currentFrame = (m_currentFrame + 1) % MaxFrameDraws;
    }

    void Renderer::WaitUntilIdle()
    {
        if (m_device)
        {
            m_device->WaitUntilIdle();
        }
    }

    void Renderer::AddObject(Object* object)
    {
        m_objects.insert(object);
    }

    void Renderer::RemoveObject(Object* object)
    {
        m_objects.erase(object);
    }

    void Renderer::RecordCommands()
    {
        const uint32_t imageCount = m_swapChain->GetImageCount();
        for (uint32_t imageIndex = 0; imageIndex < imageCount; ++imageIndex)
        {
            Vulkan::CommandBuffer* commandBuffer = m_swapChain->GetCommandBuffer(imageIndex);

            if (commandBuffer->Begin())
            {
                commandBuffer->BeginRenderPass(
                    m_swapChain->GetFrameBuffer(imageIndex), 
                    Math::CreateColor(Math::Colors::SteelBlue.xyz() * 0.7f));

                commandBuffer->BindPipeline(m_pipeline.get());

                for (auto* object : m_objects)
                {
                    // Bind Vertex and Index Buffers
                    commandBuffer->BindVertexBuffers({ object->GetVertexBuffer().get() });
                    commandBuffer->BindIndexBuffer(object->GetIndexBuffer().get());

                    // Draw
                    commandBuffer->DrawIndexed(object->GetIndexCount());
                }

                commandBuffer->EndRenderPass();
                commandBuffer->End();
            }
        }
    }

    bool Renderer::CreateInstance()
    {
        m_instance = std::make_unique<Vulkan::Instance>(m_window->GetWindowHandler());

        if (!m_instance->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create instance.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateDevice()
    {
        m_device = std::make_unique<Vulkan::Device>(m_instance.get());

        if (!m_device->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create device.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateSwapChain()
    {
        m_swapChain = std::make_unique<Vulkan::SwapChain>(m_device.get());

        if (!m_swapChain->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create swap chain.");
            return false;
        }

        DX_ASSERT(MaxFrameDraws < m_swapChain->GetImageCount(), "Renderer",
            "MaxFrameDraws (%d) is greater or equal than swap chain's images (%d).",
            MaxFrameDraws, m_swapChain->GetImageCount());

        return true;
    }

    bool Renderer::CreatePipeline()
    {
        const Math::Rectangle viewport(
            Math::Vector2(0.0f, 0.0f),
            Math::Vector2(m_swapChain->GetImageSize()));

        m_pipeline = std::make_unique<Vulkan::Pipeline>(m_device.get(), m_swapChain->GetImageFormat(), viewport);

        if (!m_pipeline->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create pipeline.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateFrameBuffers()
    {
        if (!m_swapChain->CreateFrameBuffers(m_pipeline->GetVkRenderPass()))
        {
            DX_LOG(Error, "Renderer", "Failed to create frame buffers for the swap chain.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateSynchronisation()
    {
        VkSemaphoreCreateInfo vkSemaphoreCreateInfo = {};
        vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkSemaphoreCreateInfo.pNext = nullptr;
        vkSemaphoreCreateInfo.flags = 0;

        VkFenceCreateInfo vkFenceCreateInfo = {};
        vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkFenceCreateInfo.pNext = nullptr;
        vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start fence signaled

        for (int i = 0; i < MaxFrameDraws; ++i)
        {
            if (vkCreateSemaphore(m_device->GetVkDevice(), &vkSemaphoreCreateInfo,
                nullptr, &m_vkImageAvailableSemaphores[i]) != VK_SUCCESS)
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan image available semaphore.");
                return false;
            }

            if (vkCreateSemaphore(m_device->GetVkDevice(), &vkSemaphoreCreateInfo,
                nullptr, &m_vkRenderFinishedSemaphores[i]) != VK_SUCCESS)
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan render finished semaphore.");
                return false;
            }

            if (vkCreateFence(m_device->GetVkDevice(), &vkFenceCreateInfo,
                nullptr, &m_vkRenderFences[i]) != VK_SUCCESS)
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan render fence.");
                return false;
            }
        }

        return true;
    }
} // namespace DX
