#include <Renderer/Renderer.h>

#include <Renderer/Vulkan/Instance.h>
#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/SwapChain.h>

#include <Log/Log.h>
#include <Debug/Debug.h>
#include <File/FileUtils.h>

// Necessary to ask GLFW to create a Vulkan surface for the window
#define GLFW_INCLUDE_VULKAN // This will cause glfw3.h to include vulkan.h already
#include <GLFW/glfw3.h>

namespace DX
{
    namespace Utils
    {
        bool CreateVkShaderModule(VkDevice vkDevice, const std::vector<uint8_t>& shaderByteCode, VkShaderModule* vkShaderModuleOut)
        {
            VkShaderModuleCreateInfo vkShaderModuleCreateInfo = {};
            vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            vkShaderModuleCreateInfo.pNext = nullptr;
            vkShaderModuleCreateInfo.flags = 0;
            vkShaderModuleCreateInfo.codeSize = shaderByteCode.size();
            vkShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderByteCode.data());

            if (vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, nullptr, vkShaderModuleOut) != VK_SUCCESS)
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan Shader Module.");
                return false;
            }

            return true;
        }
    }

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

        if (!CreateVkSurface())
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

        if (!CreateGraphicsPipeline())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Renderer::Terminate()
    {
        DX_LOG(Info, "Renderer", "Terminating Renderer...");

        m_window->UnregisterWindowResizeEvent(m_windowResizeHandler);

        m_swapChain.reset();
        m_device.reset();
        vkDestroySurfaceKHR((m_instance) ? m_instance->GetVkInstance() : nullptr, m_vkSurface, nullptr);
        m_vkSurface = nullptr;
        m_instance.reset();
    }

    Window* Renderer::GetWindow()
    {
        return m_window;
    }

    bool Renderer::CreateInstance()
    {
        m_instance = std::make_unique<Vulkan::Instance>();

        if (!m_instance->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create instance.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateVkSurface()
    {
        // It uses GLFW library to create the Vulkan surface for the window it handles.
        // The surface creates must match the operative system, for example on Windows
        // it will use vkCreateWin32SurfaceKHR. GLFW handles this automatically and creates
        // the appropriate Vulkan surface.
        if (glfwCreateWindowSurface(m_instance->GetVkInstance(), m_window->GetWindowHandler(), nullptr, &m_vkSurface) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to create Vulkan surface for window.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateDevice()
    {
        m_device = std::make_unique<Vulkan::Device>(m_instance.get(), m_vkSurface);

        if (!m_device->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create device.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateSwapChain()
    {
        // Get frame buffer size in pixels from GLWF
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window->GetWindowHandler(), &width, &height);

        m_swapChain = std::make_unique<Vulkan::SwapChain>(m_device.get(), Math::Vector2Int(width, height));

        if (!m_swapChain->Initialize())
        {
            DX_LOG(Error, "Renderer", "Failed to create swap chain.");
            return false;
        }

        return true;
    }

    bool Renderer::CreateGraphicsPipeline()
    {
        const char* vertexShaderFilename = "Shaders/vert.spv";
        const char* fragmentShaderFilename = "Shaders/frag.spv";

        // Read Shader ByteCode (SPIR-V)
        const auto vertexShaderByteCode = ReadAssetBinaryFile(vertexShaderFilename);
        if (!vertexShaderByteCode.has_value())
        {
            DX_LOG(Error, "Renderer", "Failed to read vertex shader file %s.", vertexShaderFilename);
            return false;
        }

        const auto fragmentShaderByteCode = ReadAssetBinaryFile(fragmentShaderFilename);
        if (!vertexShaderByteCode.has_value())
        {
            DX_LOG(Error, "Renderer", "Failed to read fragment shader file %s.", fragmentShaderFilename);
            return false;
        }

        // Create Shader Models
        VkShaderModule vertexShaderModule = nullptr;
        if (!Utils::CreateVkShaderModule(m_device->GetVkDevice(), vertexShaderByteCode.value(), &vertexShaderModule) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to create Vulkan vertex shader module for shader %s.", vertexShaderFilename);
            return false;
        }

        VkShaderModule framentShaderModule = nullptr;
        if (!Utils::CreateVkShaderModule(m_device->GetVkDevice(), fragmentShaderByteCode.value(), &framentShaderModule) != VK_SUCCESS)
        {
            DX_LOG(Error, "Renderer", "Failed to create Vulkan fragment shader modul for shader %s.", fragmentShaderFilename);
            return false;
        }

        // Create Pipeline

        // Destroy Shader Models
        // Once the pipeline object is created it will contain the shaders,
        // so the shader module objects are no longer needed and can be destroyed.
        vkDestroyShaderModule(m_device->GetVkDevice(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_device->GetVkDevice(), framentShaderModule, nullptr);
        vertexShaderModule = nullptr;
        framentShaderModule = nullptr;

        return true;
    }
} // namespace DX
