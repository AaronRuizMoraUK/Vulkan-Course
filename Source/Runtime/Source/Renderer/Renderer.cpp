#include <Renderer/Renderer.h>

#include <Renderer/Object.h>
#include <RHI/Device/Instance.h>
#include <RHI/Device/Device.h>
#include <RHI/SwapChain/SwapChain.h>
#include <RHI/Pipeline/Pipeline.h>
#include <RHI/Pipeline/PipelineDescriptorSet.h>
#include <RHI/CommandBuffer/CommandBuffer.h>
#include <RHI/Resource/Buffer/Buffer.h>

#include <Camera/Camera.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace DX
{
    Renderer::Renderer(RendererId rendererId, Window* window)
        : m_rendererId(rendererId)
        , m_window(window)
    {
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

        if (!CreateFrameData())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Renderer::Terminate()
    {
        // Necessary before destroying synchronization and frame data
        WaitUntilIdle();

        DX_LOG(Info, "Renderer", "Terminating Renderer...");

        m_perSceneDescritorSets.clear();
        m_viewProjUniformBuffers.clear();
        m_commandBuffers.clear();

        if (m_device)
        {
            std::ranges::for_each(m_vkImageAvailableSemaphores, [this](VkSemaphore vkSemaphore)
                {
                    vkDestroySemaphore(m_device->GetVkDevice(), vkSemaphore, nullptr);
                });
            std::ranges::for_each(m_vkRenderFinishedSemaphores, [this](VkSemaphore vkSemaphore)
                {
                    vkDestroySemaphore(m_device->GetVkDevice(), vkSemaphore, nullptr);
                });
            std::ranges::for_each(m_vkRenderFences, [this](VkFence vkFence)
                {
                    vkDestroyFence(m_device->GetVkDevice(), vkFence, nullptr);
                });
        }
        m_vkImageAvailableSemaphores.clear();
        m_vkRenderFinishedSemaphores.clear();
        m_vkRenderFences.clear();

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

        // 2) Update data and record the commands for the current frame
        UpdateFrameData();
        RecordCommands(m_swapChain->GetFrameBuffer(swapChainImageIndex));

        // 3) Submit the command buffer (of the current image) to the queue for execution.
        //    Wait at the convenient stage within the pipeline for the image semaphore to be signaled (so it's available for drawing to it).
        //    For example, allow to execute vertex shader, but wait for the image semaphore to be available before executing fragment shader.
        //    Lastly signal (with a different semaphore) when it has finished rendering.
        const std::vector<VkPipelineStageFlags> waitStages = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
        VkCommandBuffer currentCommandBuffer = m_commandBuffers[m_currentFrame]->GetVkCommandBuffer();

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
        
        // 4) Present image to screen when it has signaled that it has finished rendering.
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
        m_currentFrame = (m_currentFrame + 1) % Vulkan::MaxFrameDraws;
    }

    void Renderer::WaitUntilIdle()
    {
        if (m_device)
        {
            m_device->WaitUntilIdle();
        }
    }

    void Renderer::SetCamera(Camera* camera)
    {
        m_camera = camera;
    }

    void Renderer::AddObject(Object* object)
    {
        m_objects.insert(object);
    }

    void Renderer::RemoveObject(Object* object)
    {
        m_objects.erase(object);
    }

    void Renderer::UpdateFrameData()
    {
        // Update ViewProj uniform buffer
        const ViewProjBuffer viewProjBuffer(
            m_camera->GetViewMatrix(),
            m_camera->GetProjectionMatrix(),
            Math::Vector4{ m_camera->GetTransform().m_position, 1.0f }
        );
        m_viewProjUniformBuffers[m_currentFrame]->UpdateBufferData(&viewProjBuffer, sizeof(viewProjBuffer));
    }

    void Renderer::RecordCommands(Vulkan::FrameBuffer* frameBuffer)
    {
        // Calling vkResetCommandPool before reusing its command buffers in this frame.
        // Otherwise, the pool will keep on growing until you run out of memory.
        m_device->ResetVkCommandPool(Vulkan::QueueFamilyType_Graphics, m_currentFrame);

        // When Vulkan Validation is enabled, resetting the command pool or queues is not enough
        // to free memory. The driver is keeping the memory for debugging and tracking purposes.
        // Recreating (which actually reallocates) the command buffers in this case forces to
        // free the memory and avoid the application's memory to continuously grow every frame.
        if (Vulkan::Validation::DebugEnabled)
        {
            m_commandBuffers[m_currentFrame] = std::make_unique<Vulkan::CommandBuffer>(m_device.get(),
                m_device->GetVkCommandPool(Vulkan::QueueFamilyType_Graphics, m_currentFrame));
            const bool ok = m_commandBuffers[m_currentFrame]->Initialize();
            DX_ASSERT(ok, "Renderer", "Failed to recreate command buffer");
        }

        Vulkan::CommandBuffer* commandBuffer = m_commandBuffers[m_currentFrame].get();

        if (commandBuffer->Begin())
        {
            commandBuffer->BeginRenderPass(frameBuffer,
                Math::CreateColor(Math::Colors::SteelBlue.xyz() * 0.7f));

            commandBuffer->BindPipeline(m_pipeline.get());

            // Bind per scene pipeline descriptor set, which includes the ViewProj uniform buffer.
            commandBuffer->BindPipelineDescriptorSet(m_perSceneDescritorSets[m_currentFrame].get());

            for (uint32_t objectIndex = 0;
                auto* object : m_objects)
            {
                // Push per object World data to the pipeline.
                const WorldBuffer worldBuffer = {
                    .m_worldMatrix = object->GetTransform().ToMatrix(),
                    .m_inverseTransposeWorldMatrix = object->GetTransform().ToMatrix().Inverse().Transpose()
                };
                commandBuffer->PushConstantsToPipeline(
                    m_pipeline.get(), Vulkan::ShaderType_Vertex, &worldBuffer, sizeof(worldBuffer));

                // Bind Vertex and Index Buffers
                commandBuffer->BindVertexBuffers({ object->GetVertexBuffer().get() });
                commandBuffer->BindIndexBuffer(object->GetIndexBuffer().get());

                // Draw
                commandBuffer->DrawIndexed(object->GetIndexCount());

                ++objectIndex;
            }

            commandBuffer->EndRenderPass();
            commandBuffer->End();
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

        DX_ASSERT(Vulkan::MaxFrameDraws < m_swapChain->GetImageCount(), "Renderer",
            "MaxFrameDraws (%d) is greater or equal than swap chain's images (%d).",
            Vulkan::MaxFrameDraws, m_swapChain->GetImageCount());

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

        m_vkImageAvailableSemaphores.resize(Vulkan::MaxFrameDraws, nullptr);
        m_vkRenderFinishedSemaphores.resize(Vulkan::MaxFrameDraws, nullptr);
        m_vkRenderFences.resize(Vulkan::MaxFrameDraws, nullptr);

        for (int i = 0; i < Vulkan::MaxFrameDraws; ++i)
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

    bool Renderer::CreateFrameData()
    {
        // ViewProj Uniform Buffers
        Vulkan::BufferDesc viewProjBufferDesc = {};
        viewProjBufferDesc.m_elementSizeInBytes = sizeof(ViewProjBuffer);
        viewProjBufferDesc.m_elementCount = 1;
        viewProjBufferDesc.m_usageFlags = Vulkan::BufferUsage_UniformBuffer;
        viewProjBufferDesc.m_memoryProperty = Vulkan::ResourceMemoryProperty::HostVisible;
        viewProjBufferDesc.m_initialData = nullptr; // ViewProj data will be set every frame to this buffer

        m_commandBuffers.resize(Vulkan::MaxFrameDraws);
        m_viewProjUniformBuffers.resize(Vulkan::MaxFrameDraws);
        m_perSceneDescritorSets.resize(Vulkan::MaxFrameDraws);

        for (int i = 0; i < Vulkan::MaxFrameDraws; ++i)
        {
            m_commandBuffers[i] = std::make_unique<Vulkan::CommandBuffer>(m_device.get(), 
                m_device->GetVkCommandPool(Vulkan::QueueFamilyType_Graphics, i));
            if (!m_commandBuffers[i]->Initialize())
            {
                DX_LOG(Error, "Renderer", "Failed to create CommandBuffer.");
                return false;
            }

            m_viewProjUniformBuffers[i] = std::make_unique<Vulkan::Buffer>(m_device.get(), viewProjBufferDesc);
            if (!m_viewProjUniformBuffers[i]->Initialize())
            {
                DX_LOG(Error, "Renderer", "Failed to create uniform buffer for ViewProj data.");
                return false;
            }

            m_perSceneDescritorSets[i] = m_pipeline->CreatePipelineDescriptorSet(0);
            if (!m_perSceneDescritorSets[i]->Initialize())
            {
                return false;
            }

            // Fill the Pipeline Descriptor Sets with the Uniform Buffers
            // ViewProj uniform buffer is in layout binding 0, which internally points to shader resource binding 0.
            m_perSceneDescritorSets[i]->SetUniformBuffer(0, m_viewProjUniformBuffers[i].get());
        }

        return true;
    }
} // namespace DX
