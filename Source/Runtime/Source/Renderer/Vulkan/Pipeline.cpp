#include <Renderer/Vulkan/Pipeline.h>

#include <Renderer/Vulkan/Device.h>

#include <Log/Log.h>
#include <Debug/Debug.h>
#include <File/FileUtils.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    namespace Utils
    {
        bool CreateVkShaderModule(VkDevice vkDevice, const std::vector<uint8_t>& shaderByteCode, VkShaderModule* vkShaderModuleOut)
        {
            if (shaderByteCode.size() % sizeof(uint32_t) != 0)
            {
                DX_LOG(Error, "Renderer", "Shader SPIR-V byte code size is not a multiple of 4 bytes");
                return false;
            }

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
    } // namespace Utils

    Pipeline::Pipeline(Device* device)
        : m_device(device)
    {
    }

    Pipeline::~Pipeline()
    {
        Terminate();
    }

    bool Pipeline::Initialize()
    {
        //if (m_vkInstance)
        //{
        //    return true; // Already initialized
        //}

        DX_LOG(Info, "Vulkan Pipeline", "Initializing Vulkan Pipeline...");

        if (!CreateVkPipeline())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Pipeline::Terminate()
    {
        DX_LOG(Info, "Vulkan Pipeline", "Terminating Vulkan Pipeline...");

    }

    bool Pipeline::CreateVkPipeline()
    {
        const char* vertexShaderFilename = "Shaders/vert.spv";
        const char* fragmentShaderFilename = "Shaders/frag.spv";

        // Read Shader ByteCode (SPIR-V)
        const auto vertexShaderByteCode = DX::ReadAssetBinaryFile(vertexShaderFilename);
        if (!vertexShaderByteCode.has_value())
        {
            DX_LOG(Error, "Renderer", "Failed to read vertex shader file %s.", vertexShaderFilename);
            return false;
        }

        const auto fragmentShaderByteCode = DX::ReadAssetBinaryFile(fragmentShaderFilename);
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
        {
            // Pipeline Shader Stages
            std::array<VkPipelineShaderStageCreateInfo, 2> vkPipelineShaderStagesCreateInfo;

            vkPipelineShaderStagesCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vkPipelineShaderStagesCreateInfo[0].pNext = nullptr;
            vkPipelineShaderStagesCreateInfo[0].flags = 0;
            vkPipelineShaderStagesCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            vkPipelineShaderStagesCreateInfo[0].module = vertexShaderModule;
            vkPipelineShaderStagesCreateInfo[0].pName = "main";
            vkPipelineShaderStagesCreateInfo[0].pSpecializationInfo = nullptr;

            vkPipelineShaderStagesCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vkPipelineShaderStagesCreateInfo[1].pNext = nullptr;
            vkPipelineShaderStagesCreateInfo[1].flags = 0;
            vkPipelineShaderStagesCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            vkPipelineShaderStagesCreateInfo[1].module = framentShaderModule;
            vkPipelineShaderStagesCreateInfo[1].pName = "main";
            vkPipelineShaderStagesCreateInfo[1].pSpecializationInfo = nullptr;


        }

        // Destroy Shader Models
        // Once the pipeline object is created it will contain the shaders,
        // so the shader module objects are no longer needed and can be destroyed.
        vkDestroyShaderModule(m_device->GetVkDevice(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_device->GetVkDevice(), framentShaderModule, nullptr);
        vertexShaderModule = nullptr;
        framentShaderModule = nullptr;

        return true;
    }
} // namespace Vulkan
