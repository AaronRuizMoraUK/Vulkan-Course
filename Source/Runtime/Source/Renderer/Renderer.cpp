#include <Renderer/Renderer.h>

#include <Renderer/Vulkan/Instance.h>
#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/SwapChain.h>
#include <Renderer/Vulkan/Pipeline.h>
#include <Renderer/Vulkan/CommandBuffer.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

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

        if (!CreateCommandBuffers())
        {
            Terminate();
            return false;
        }

        // Pre-record the commands in all command buffers of the swap chain
        // TODO: to be removed and generate the command to only the command buffer
        //       of the current frame buffer being rendered to.
        RecordCommands();

        return true;
    }

    void Renderer::Terminate()
    {
        DX_LOG(Info, "Renderer", "Terminating Renderer...");

        m_pipeline.reset();
        m_swapChain.reset();
        m_device.reset();
        m_instance.reset();
    }

    Window* Renderer::GetWindow()
    {
        return m_window;
    }

    void Renderer::RecordCommands()
    {
        const uint32_t imageCount = m_swapChain->GetImageCount();
        for (uint32_t imageIndex = 0; imageIndex < imageCount; ++imageIndex)
        {
            auto* commandBuffer = m_swapChain->GetCommandBuffer(imageIndex);

            if (commandBuffer->Begin())
            {
                commandBuffer->BeginRenderPass(
                    m_swapChain->GetFrameBuffer(imageIndex), 
                    Math::CreateColor(Math::Colors::SteelBlue.xyz() * 0.7f));
                {
                    commandBuffer->BindPipeline(m_pipeline.get());

                    commandBuffer->Draw(3);
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

    bool Renderer::CreateCommandBuffers()
    {
        if (!m_swapChain->CreateCommandBuffers())
        {
            DX_LOG(Error, "Renderer", "Failed to create command buffers for the swap chain's frame buffers.");
            return false;
        }

        return true;
    }
} // namespace DX
