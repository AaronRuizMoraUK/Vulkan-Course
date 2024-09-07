#include <Renderer/Renderer.h>

#include <Renderer/Object.h>
#include <RHI/Device/Instance.h>
#include <RHI/Device/Device.h>
#include <RHI/SwapChain/SwapChain.h>
#include <RHI/RenderPass/RenderPass.h>
#include <RHI/Pipeline/Pipeline.h>
#include <RHI/Pipeline/PipelineDescriptorSet.h>
#include <RHI/CommandBuffer/CommandBuffer.h>
#include <RHI/Resource/Buffer/Buffer.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Resource/ImageView/ImageView.h>
#include <RHI/FrameBuffer/FrameBuffer.h>

#include <Camera/Camera.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

// TODO: To be removed. Required for VkSemaphore, VkFence, VkCommandPool and VkFormatFeatureFlagBits used.
#include <vulkan/vulkan.h>

namespace DX
{
    Renderer::Renderer(RendererId rendererId, Window* window)
        : m_rendererId(rendererId)
        , m_window(window)
        , m_frameBufferColorFormat(Vulkan::ResourceFormat::Unknown)
        , m_frameBufferDepthStencilFormat(Vulkan::ResourceFormat::Unknown)
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

        if (!CreateRenderPass())
        {
            Terminate();
            return false;
        }

        if (!CreateFrameBuffers())
        {
            Terminate();
            return false;
        }

        if (!CreatePipelines())
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

        m_inputAttachmentsDescritorSets.clear();
        m_perObjectDescritorSets.clear();
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

        m_pipelines.clear();
        m_frameBuffers.clear();
        m_renderPass.reset();
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
        UpdateFrameData(m_frameBuffers[swapChainImageIndex].get());
        RecordCommands(m_frameBuffers[swapChainImageIndex].get());

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

    void Renderer::UpdateFrameData(Vulkan::FrameBuffer* frameBuffer)
    {
        // Update ViewProj uniform buffer
        const ViewProjBuffer viewProjBuffer(
            m_camera->GetViewMatrix(),
            m_camera->GetProjectionMatrix(),
            Math::Vector4{ m_camera->GetTransform().m_position, 1.0f }
        );
        m_viewProjUniformBuffers[m_currentFrame]->UpdateBufferData(&viewProjBuffer, sizeof(viewProjBuffer));

        // Update per object pipeline descriptor set
        for (uint32_t objectIndex = 0;
            auto * object : m_objects)
        {
            Vulkan::PipelineDescriptorSet* objectDescriptorSet = m_perObjectDescritorSets[m_currentFrame][objectIndex].get();

            objectDescriptorSet->SetShaderSampler(0, object->GetSampler().get());
            objectDescriptorSet->SetShaderSampledImageView(1, object->GetDiffuseImageView().get());
            objectDescriptorSet->SetShaderSampledImageView(2, object->GetEmissiveImageView().get());
            objectDescriptorSet->SetShaderSampledImageView(3, object->GetNormalImageView().get());

            ++objectIndex;
        }

        // Update the Pipeline Descriptor Sets with the Input Attachments
        Vulkan::ImageView* colorAttachment = frameBuffer->GetImageView(1);
        Vulkan::ImageView* depthStencilAttachment = frameBuffer->GetImageView(2);
        m_inputAttachmentsDescritorSets[m_currentFrame]->SetShaderInputAttachment(0, colorAttachment);
        m_inputAttachmentsDescritorSets[m_currentFrame]->SetShaderInputAttachment(1, depthStencilAttachment);
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
                Math::CreateColor(Math::Colors::SteelBlue.xyz() * 0.7f),
                1.0f);

            // Subpass 0
            {
                commandBuffer->BindPipeline(m_pipelines[0].get());

                // Bind per scene pipeline descriptor set, which includes the ViewProj uniform buffer.
                commandBuffer->BindPipelineDescriptorSet(m_perSceneDescritorSets[m_currentFrame].get());

                for (uint32_t objectIndex = 0;
                    auto* object : m_objects)
                {
                    // Bind per object pipeline descriptor set, which includes the images and sampler.
                    commandBuffer->BindPipelineDescriptorSet(m_perObjectDescritorSets[m_currentFrame][objectIndex].get());

                    // Push per object World data to the pipeline.
                    const WorldBuffer worldBuffer = {
                        .m_worldMatrix = object->GetTransform().ToMatrix(),
                        .m_inverseTransposeWorldMatrix = object->GetTransform().ToMatrix().Inverse().Transpose()
                    };
                    commandBuffer->PushConstantsToPipeline(
                        m_pipelines[0].get(), Vulkan::ShaderType_Vertex | Vulkan::ShaderType_Fragment, &worldBuffer, sizeof(worldBuffer));

                    // Bind Vertex and Index Buffers
                    commandBuffer->BindVertexBuffers({ object->GetVertexBuffer().get() });
                    commandBuffer->BindIndexBuffer(object->GetIndexBuffer().get());

                    // Draw
                    commandBuffer->DrawIndexed(object->GetIndexCount());

                    ++objectIndex;
                }
            }

            commandBuffer->NextSubpass();

            // Subpass 1
            {
                commandBuffer->BindPipeline(m_pipelines[1].get());

                // Bind input attachments pipeline descriptor set, which includes the color and depth input images.
                commandBuffer->BindPipelineDescriptorSet(m_inputAttachmentsDescritorSets[m_currentFrame].get());

                // Draw 3 vertices, the vertex positions are handled by the vertex shader, there is no vertex data to bind.
                commandBuffer->Draw(3);
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

        return true;
    }

    bool Renderer::CreateRenderPass()
    {
        // Choose the most appropriate color format
        m_frameBufferColorFormat = Vulkan::ChooseSupportedFormat(m_device.get(),
            { Vulkan::ResourceFormat::R8G8B8A8_UNORM },
            Vulkan::ImageTiling::Optimal,
            VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
        );
        if (m_frameBufferColorFormat == Vulkan::ResourceFormat::Unknown)
        {
            DX_LOG(Error, "Renderer", "Failed to find a supported color format for frame buffer.");
            return false;
        }

        // Choose the most appropriate depth format
        m_frameBufferDepthStencilFormat = Vulkan::ChooseSupportedFormat(m_device.get(),
            { Vulkan::ResourceFormat::D32_SFLOAT_S8_UINT, Vulkan::ResourceFormat::D24_UNORM_S8_UINT },
            Vulkan::ImageTiling::Optimal,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
        if (m_frameBufferDepthStencilFormat == Vulkan::ResourceFormat::Unknown)
        {
            DX_LOG(Error, "Renderer", "Failed to find a supported depth-stencil format for frame buffer.");
            return false;
        }

        Vulkan::RenderPassDesc renderPassDesc = {};
        renderPassDesc.m_attachments = {
            m_swapChain->GetImageFormat(),
            m_frameBufferColorFormat,
            m_frameBufferDepthStencilFormat
        };

        m_renderPass = std::make_unique<Vulkan::RenderPass>(m_device.get(), renderPassDesc);

        if (!m_renderPass->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create render pass.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateFrameBuffers()
    {
        std::vector<std::shared_ptr<Vulkan::Image>> swapChainImages = m_swapChain->ObtainImagesFromSwapChain();
        if (swapChainImages.empty())
        {
            DX_LOG(Error, "Renderer", "Failed to obtain Vulkan SwapChain Images.");
            return false;
        }

        m_frameBuffers.resize(swapChainImages.size());
        for (int i = 0; i < swapChainImages.size(); ++i)
        {
            // Create Color Image
            std::shared_ptr<Vulkan::Image> colorImage;
            {
                Vulkan::ImageDesc colorImageDesc = {};
                colorImageDesc.m_imageType = Vulkan::ImageType::Image2D;
                colorImageDesc.m_dimensions = Math::Vector3Int(m_swapChain->GetImageSize(), 1);
                colorImageDesc.m_mipCount = 1;
                colorImageDesc.m_format = m_frameBufferColorFormat;
                colorImageDesc.m_tiling = Vulkan::ImageTiling::Optimal;
                colorImageDesc.m_usageFlags = Vulkan::ImageUsage_ColorAttachment | Vulkan::ImageUsage_InputAttachment;

                colorImage = std::make_shared<Vulkan::Image>(m_device.get(), colorImageDesc);
                if (!colorImage->Initialize())
                {
                    DX_LOG(Error, "Renderer", "Failed to create Vulkan Image for Color Attachment.");
                    return false;
                }
            }

            // Create DepthStencil Image
            std::shared_ptr<Vulkan::Image> depthStencilImage;
            {
                Vulkan::ImageDesc depthStencilImageDesc = {};
                depthStencilImageDesc.m_imageType = Vulkan::ImageType::Image2D;
                depthStencilImageDesc.m_dimensions = Math::Vector3Int(m_swapChain->GetImageSize(), 1);
                depthStencilImageDesc.m_mipCount = 1;
                depthStencilImageDesc.m_format = m_frameBufferDepthStencilFormat;
                depthStencilImageDesc.m_tiling = Vulkan::ImageTiling::Optimal;
                depthStencilImageDesc.m_usageFlags = Vulkan::ImageUsage_DepthStencilAttachment | Vulkan::ImageUsage_InputAttachment;

                depthStencilImage = std::make_shared<Vulkan::Image>(m_device.get(), depthStencilImageDesc);
                if (!depthStencilImage->Initialize())
                {
                    DX_LOG(Error, "Renderer", "Failed to create Vulkan Image for Depth Attachment.");
                    return false;
                }
            }

            // Create Frame Buffer
            {
                Vulkan::FrameBufferDesc frameBufferDesc = {};
                frameBufferDesc.m_renderPass = m_renderPass.get();
                frameBufferDesc.m_attachments = {
                    {
                        swapChainImages[i],
                        swapChainImages[i]->GetImageDesc().m_format,
                        Vulkan::ImageViewAspect_Color
                    },
                    {
                        colorImage,
                        colorImage->GetImageDesc().m_format,
                        Vulkan::ImageViewAspect_Color
                    },
                    {
                        depthStencilImage,
                        depthStencilImage->GetImageDesc().m_format,
                        // NOTE: This attachment is used by subpass 0 as output depth attachment and
                        //       by subpass 1 as input attachment. When used as input attachment in subpass 1
                        //       to be read by shader, the view cannot have both depth and stencil aspects.
                        Vulkan::ImageViewAspect_Depth //| Vulkan::ImageViewAspect_Stencil
                    }
                };

                m_frameBuffers[i] = std::make_unique<Vulkan::FrameBuffer>(m_device.get(), frameBufferDesc);
                if (!m_frameBuffers[i]->Initialize())
                {
                    DX_LOG(Error, "Renderer", "Failed to create FrameBuffer.");
                    return false;
                }
            }
        }

        return true;
    }

    bool Renderer::CreatePipelines()
    {
        const Math::Rectangle viewport(
            Math::Vector2(0.0f, 0.0f),
            Math::Vector2(m_swapChain->GetImageSize()));

        m_pipelines.resize(2); // 2 pipelines, one for each subpass

        // Subpass 0
        m_pipelines[0] = std::make_unique<Vulkan::Pipeline>(m_device.get(), m_renderPass.get(), 0, viewport);
        if (!m_pipelines[0]->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create pipeline.");
            return false;
        }

        // Subpass 1
        m_pipelines[1] = std::make_unique<Vulkan::Pipeline>(m_device.get(), m_renderPass.get(), 1, viewport);
        if (!m_pipelines[1]->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create pipeline.");
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
        m_perObjectDescritorSets.resize(Vulkan::MaxFrameDraws);
        m_inputAttachmentsDescritorSets.resize(Vulkan::MaxFrameDraws);

        for (int i = 0; i < Vulkan::MaxFrameDraws; ++i)
        {
            m_commandBuffers[i] = std::make_unique<Vulkan::CommandBuffer>(m_device.get(), 
                m_device->GetVkCommandPool(Vulkan::QueueFamilyType_Graphics, i));
            if (!m_commandBuffers[i]->Initialize())
            {
                DX_LOG(Error, "Renderer", "Failed to create CommandBuffer.");
                return false;
            }

            // Per Scene resources (Subpass 0)
            {
                m_viewProjUniformBuffers[i] = std::make_unique<Vulkan::Buffer>(m_device.get(), viewProjBufferDesc);
                if (!m_viewProjUniformBuffers[i]->Initialize())
                {
                    DX_LOG(Error, "Renderer", "Failed to create uniform buffer for ViewProj data.");
                    return false;
                }

                constexpr uint32_t descriptorSetIndexForPerSceneResources = 0;
                m_perSceneDescritorSets[i] = m_pipelines[0]->CreatePipelineDescriptorSet(descriptorSetIndexForPerSceneResources);
                if (!m_perSceneDescritorSets[i])
                {
                    return false;
                }

                // Fill the Pipeline Descriptor Sets with the Uniform Buffers
                // ViewProj uniform buffer is in layout binding 0, which internally points to shader resource binding 0.
                m_perSceneDescritorSets[i]->SetShaderUniformBuffer(0, m_viewProjUniformBuffers[i].get());
            }

            // Per Object resources (Subpass 0)
            {
                m_perObjectDescritorSets[i].resize(Vulkan::MaxObjects);
                for (int objectIndex = 0; objectIndex < Vulkan::MaxObjects; ++objectIndex)
                {
                    constexpr uint32_t descriptorSetIndexForPerObjectResources = 1;
                    m_perObjectDescritorSets[i][objectIndex] = m_pipelines[0]->CreatePipelineDescriptorSet(descriptorSetIndexForPerObjectResources);
                    if (!m_perObjectDescritorSets[i][objectIndex])
                    {
                        return false;
                    }
                }

                // Per object descriptor sets will be filled with data every frame
            }

            // Input Attachments (Subpass 1)
            {
                constexpr uint32_t descriptorSetIndexForInputAttachments = 0;
                m_inputAttachmentsDescritorSets[i] = m_pipelines[1]->CreatePipelineDescriptorSet(descriptorSetIndexForInputAttachments);
                if (!m_inputAttachmentsDescritorSets[i])
                {
                    return false;
                }

                // Input Attachments descriptor sets will be filled with data every frame

            }
        }

        return true;
    }
} // namespace DX
