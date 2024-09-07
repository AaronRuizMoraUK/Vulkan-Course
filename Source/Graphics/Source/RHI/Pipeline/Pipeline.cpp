#include <RHI/Pipeline/Pipeline.h>

#include <RHI/Device/Device.h>
#include <RHI/RenderPass/RenderPass.h>
#include <RHI/Pipeline/PipelineDescriptorSet.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>
#include <File/FileUtils.h>
#include <Math/Matrix4x4.h>

#include <vulkan/vulkan.h>

#include <numeric>

namespace Vulkan
{
    namespace Utils
    {
        // Helper structure to destroy shader module when it gets out of scope.
        struct ScopedShaderModule
        {
            VkShaderModule m_vkShaderModule = nullptr;

            ScopedShaderModule(Device* device)
                : m_device(device)
            {
            }

            ~ScopedShaderModule()
            {
                vkDestroyShaderModule(m_device->GetVkDevice(), m_vkShaderModule, nullptr);
            }

        private:
            Device* m_device = nullptr;
        };

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

        bool IsDescriptorTypeDynamic(VkDescriptorType type)
        {
            return type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        }

        uint32_t GetDynamicDescritorCount(const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings)
        {
            return std::reduce(descriptorSetLayoutBindings.begin(), descriptorSetLayoutBindings.end(), 0u,
                [](uint32_t accumulator, const VkDescriptorSetLayoutBinding& binding)
                {
                    return accumulator +
                        IsDescriptorTypeDynamic(binding.descriptorType) ? binding.descriptorCount : 0u;
                });
        }
    } // namespace Utils

    Pipeline::Pipeline(Device* device, RenderPass* renderPass, uint32_t subpassIndex, const Math::Rectangle& viewport)
        : m_device(device)
        , m_renderPass(renderPass)
        , m_subpassIndex(subpassIndex)
        , m_viewport(viewport)
    {
    }

    Pipeline::~Pipeline()
    {
        Terminate();
    }

    bool Pipeline::Initialize()
    {
        if (m_vkPipeline)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan Pipeline", "Initializing Vulkan Pipeline...");

        if (m_subpassIndex == 0)
        {
            if (!CreateVkPipelineLayoutSubpass0())
            {
                Terminate();
                return false;
            }

            if (!CreateVkPipelineSubpass0())
            {
                Terminate();
                return false;
            }
        }
        else if (m_subpassIndex == 1)
        {
            if (!CreateVkPipelineLayoutSubpass1())
            {
                Terminate();
                return false;
            }

            if (!CreateVkPipelineSubpass1())
            {
                Terminate();
                return false;
            }
        }
        else
        {
            DX_LOG(Fatal, "Vulkan Pipeline", "Subpass index is %d and must be 0 or 1.", m_subpassIndex);
            return false;
        }

        return true;
    }

    void Pipeline::Terminate()
    {
        DX_LOG(Info, "Vulkan Pipeline", "Terminating Vulkan Pipeline...");

        vkDestroyPipeline(m_device->GetVkDevice(), m_vkPipeline, nullptr);
        m_vkPipeline = nullptr;

        vkDestroyPipelineLayout(m_device->GetVkDevice(), m_vkPipelineLayout, nullptr);
        m_vkPipelineLayout = nullptr;

        std::ranges::for_each(m_descriptorSetLayouts, [this](std::unique_ptr<DescriptorSetLayout>& descriptorSetLayout)
            {
                vkDestroyDescriptorSetLayout(m_device->GetVkDevice(), descriptorSetLayout->m_vkDescriptorSetLayout, nullptr);
            });
        m_descriptorSetLayouts.clear();
    }

    RenderPass* Pipeline::GetRenderPass()
    {
        return m_renderPass;
    }

    uint32_t Pipeline::GetSubpassIndex() const
    {
        return m_subpassIndex;
    }

    VkPipeline Pipeline::GetVkPipeline()
    {
        return m_vkPipeline;
    }

    VkPipelineLayout Pipeline::GetVkPipelineLayout()
    {
        return m_vkPipelineLayout;
    }

    DescriptorSetLayout* Pipeline::GetPipelineDescriptorSetLayout(uint32_t setLayoutIndex)
    {
        if (setLayoutIndex < m_descriptorSetLayouts.size())
        {
            return m_descriptorSetLayouts[setLayoutIndex].get();
        }
        else
        {
            return nullptr;
        }
    }

    std::shared_ptr<PipelineDescriptorSet> Pipeline::CreatePipelineDescriptorSet(uint32_t setLayoutIndex)
    {
        if (setLayoutIndex < m_descriptorSetLayouts.size())
        {
            auto descriptorSet = std::make_shared<PipelineDescriptorSet>(
                m_device, m_device->GetVkDescriptorPool(), this, setLayoutIndex);

            if (descriptorSet->Initialize())
            {
                return descriptorSet;
            }
        }
        return nullptr;
    }

    bool Pipeline::CreateVkPipelineLayoutSubpass0()
    {
        // TODO: Obtain this from the shaders.

        // Reset the descriptor set layouts
        m_descriptorSetLayouts.clear();

        // Descriptor Sets Layout 0: Per Scene resources
        {
            const std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
                // ViewProj Binding Info
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1, // Number of contiguous descriptors of this type for binding in shader
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, // Shader stage to bind to
                    .pImmutableSamplers = nullptr
                }
            };

            auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>();

            descriptorSetLayout->m_numDynamicDescriptors = Utils::GetDynamicDescritorCount(descriptorSetLayoutBindings);

            // Create Descriptor Set Layout with given bindings
            VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo = {};
            vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            vkDescriptorSetLayoutCreateInfo.pNext = nullptr;
            vkDescriptorSetLayoutCreateInfo.flags = 0;
            vkDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
            vkDescriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

            if (vkCreateDescriptorSetLayout(m_device->GetVkDevice(),
                &vkDescriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout->m_vkDescriptorSetLayout) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Pipeline", "Failed to create Vulkan Descriptor Set Layout.");
                return false;
            }

            m_descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
        }

        // Descriptor Sets Layout 1: Per Object resources
        {
            const std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
                // Sampler Binding Info
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .descriptorCount = 1, // Number of contiguous descriptors of this type for binding in shader
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // Shader stage to bind to
                    .pImmutableSamplers = nullptr
                },
                // Diffuse texture Binding Info
                {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = 1, // Number of contiguous descriptors of this type for binding in shader
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // Shader stage to bind to
                    .pImmutableSamplers = nullptr
                },
                // Emissive texture Binding Info
                {
                    .binding = 2,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = 1, // Number of contiguous descriptors of this type for binding in shader
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // Shader stage to bind to
                    .pImmutableSamplers = nullptr
                },
                // Normal texture Binding Info
                {
                    .binding = 3,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = 1, // Number of contiguous descriptors of this type for binding in shader
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // Shader stage to bind to
                    .pImmutableSamplers = nullptr
                }
            };

            auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>();

            descriptorSetLayout->m_numDynamicDescriptors = Utils::GetDynamicDescritorCount(descriptorSetLayoutBindings);

            // Create Descriptor Set Layout with given bindings
            VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo = {};
            vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            vkDescriptorSetLayoutCreateInfo.pNext = nullptr;
            vkDescriptorSetLayoutCreateInfo.flags = 0;
            vkDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
            vkDescriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

            if (vkCreateDescriptorSetLayout(m_device->GetVkDevice(),
                &vkDescriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout->m_vkDescriptorSetLayout) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Pipeline", "Failed to create Vulkan Descriptor Set Layout.");
                return false;
            }

            m_descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
        }

        // Push Constant Ranges. Maximum of 1 per shader.
        const std::vector<VkPushConstantRange> vkPushConstantRanges = {
            // Per Object World Binding Info in Vertex and Fragment Shader
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = 2 * sizeof(Math::Matrix4x4Packed) // Max size 128 bytes. It fits 2 matrices.
            }
        };

        // Pipeline Layout = Descriptor set layouts + Push constant ranges
        {
            std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts(m_descriptorSetLayouts.size(), nullptr);
            std::transform(m_descriptorSetLayouts.begin(), m_descriptorSetLayouts.end(), vkDescriptorSetLayouts.begin(),
                [](std::unique_ptr<DescriptorSetLayout>& descriptorSetLayout)
                {
                    return descriptorSetLayout->m_vkDescriptorSetLayout;
                });

            // Create Pipeline Layout (Resources Layout)
            // Where to specify Descriptor Set Layouts and Push Constant Ranges.
            VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo = {};
            vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            vkPipelineLayoutCreateInfo.pNext = nullptr;
            vkPipelineLayoutCreateInfo.flags = 0;
            vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
            vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();
            vkPipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRanges.size());
            vkPipelineLayoutCreateInfo.pPushConstantRanges = vkPushConstantRanges.data();

            if (vkCreatePipelineLayout(m_device->GetVkDevice(), &vkPipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Pipeline", "Failed to create Vulkan Pipeline Layout.");
                return false;
            }
        }

        return true;
    }

    bool Pipeline::CreateVkPipelineSubpass0()
    {
        // TODO: Pass all info to the Pipeline class, rather than generate them here.

        // Create Shader Modules
        // 
        // Once the pipeline object is created it will contain the shaders.
        // This means the shader modules will no longer be needed and need to be destroyed.
        // That is why we are using the helper structure ScopedShaderModule, to destroy the
        // shader modules when they gets out of scope of this function.
        // 
        // TODO: Pass from a configuration if the Pipeline should destroy them or not,
        //       in case they will be reused for creating other pipelines.
        Utils::ScopedShaderModule vertexShaderModule(m_device);
        Utils::ScopedShaderModule framentShaderModule(m_device);
        {
            const char* vertexShaderFilename = "Shaders/Shader.vert.spv";
            const char* fragmentShaderFilename = "Shaders/Shader.frag.spv";

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
            if (!Utils::CreateVkShaderModule(m_device->GetVkDevice(), vertexShaderByteCode.value(), &vertexShaderModule.m_vkShaderModule))
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan vertex shader module for shader %s.", vertexShaderFilename);
                return false;
            }

            if (!Utils::CreateVkShaderModule(m_device->GetVkDevice(), fragmentShaderByteCode.value(), &framentShaderModule.m_vkShaderModule))
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan fragment shader modul for shader %s.", fragmentShaderFilename);
                return false;
            }
        }

        // Create Pipeline
        {
            // Pipeline Shader Stages
            std::array<VkPipelineShaderStageCreateInfo, 2> vkPipelineShaderStagesCreateInfo;

            vkPipelineShaderStagesCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vkPipelineShaderStagesCreateInfo[0].pNext = nullptr;
            vkPipelineShaderStagesCreateInfo[0].flags = 0;
            vkPipelineShaderStagesCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            vkPipelineShaderStagesCreateInfo[0].module = vertexShaderModule.m_vkShaderModule;
            vkPipelineShaderStagesCreateInfo[0].pName = "main";
            vkPipelineShaderStagesCreateInfo[0].pSpecializationInfo = nullptr;

            vkPipelineShaderStagesCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vkPipelineShaderStagesCreateInfo[1].pNext = nullptr;
            vkPipelineShaderStagesCreateInfo[1].flags = 0;
            vkPipelineShaderStagesCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            vkPipelineShaderStagesCreateInfo[1].module = framentShaderModule.m_vkShaderModule;
            vkPipelineShaderStagesCreateInfo[1].pName = "main";
            vkPipelineShaderStagesCreateInfo[1].pSpecializationInfo = nullptr;

            // Pipeline Vertex Input State (Input Layout)
            const VkVertexInputBindingDescription vkVertexInputBindingDesc = {
                .binding = 0, // Stream
                .stride = (3 + 3 + 3 + 3 + 2) * 4, // sizeof(VertexPNTBUv)
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            };

            const std::vector<VkVertexInputAttributeDescription> vkVertexInputAttributesDesc = {
                {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},
                {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = (3) * 4 /*offsetof(VertexPNTBUv, m_normal)*/},
                {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = (3 + 3) * 4 /*offsetof(VertexPNTBUv, m_tangent)*/},
                {.location = 3, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = (3 + 3 + 3) * 4 /*offsetof(VertexPNTBUv, m_binormal)*/},
                {.location = 4, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = (3 + 3 + 3 + 3) * 4 /*offsetof(VertexPNTBUv, m_uv)*/},
            };

            VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo = {};
            vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vkPipelineVertexInputStateCreateInfo.pNext = nullptr;
            vkPipelineVertexInputStateCreateInfo.flags = 0;
            vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
            vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vkVertexInputBindingDesc; // Info about data spacing, stride info
            vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vkVertexInputAttributesDesc.size());
            vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributesDesc.data(); // Info about data format and where to bind to/from

            // Pipeline Input Assembly State (Primitive Topology)
            VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo = {};
            vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            vkPipelineInputAssemblyStateCreateInfo.pNext = nullptr;
            vkPipelineInputAssemblyStateCreateInfo.flags = 0;
            vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // When true it allows overriding of "strip" topology to start new primitives

            // Viewport & Scissor State
            // Number of viewports and scissors must match in Vulkan.
            const VkViewport vkViewport = {
                .x = m_viewport.pos.x,
                .y = m_viewport.pos.y,
                .width = m_viewport.size.x,
                .height = m_viewport.size.y,
                .minDepth = 0.0f,
                .maxDepth = 1.0f
            };

            const VkRect2D vkScissor = {
                .offset = {static_cast<int32_t>(m_viewport.pos.x), static_cast<int32_t>(m_viewport.pos.y)},
                .extent = {static_cast<uint32_t>(m_viewport.size.x), static_cast<uint32_t>(m_viewport.size.y)}
            };

            VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo = {};
            vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            vkPipelineViewportStateCreateInfo.pNext = nullptr;
            vkPipelineViewportStateCreateInfo.flags = 0;
            vkPipelineViewportStateCreateInfo.viewportCount = 1;
            vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
            vkPipelineViewportStateCreateInfo.scissorCount = 1;
            vkPipelineViewportStateCreateInfo.pScissors = &vkScissor;

            // NOTE: Setting viewport and scissor into the pipeline has the disadvantage that resizing
            // the window will require recreating the pipeline too. To better support resizing it'd be
            // better to use Dynamic States. Remember that swap chain needs to be recreated too if the
            // window is resized.
            //
            // std::vector<VkDynamicState> vkDynamicStates = {
            //    VK_DYNAMIC_STATE_VIEWPORT, // Allows to set in command buffer with vkCmdSetViewport(cmdBuffer, 0, 1, &vkViewport);
            //    VK_DYNAMIC_STATE_SCISSOR   // Can be set in command buffer with vkCmdSetScissor(cmdBuffer, 0, 1, &vkScissor);
            // };
            // 
            // VkPipelineDynamicStateCreateInfo vkPipelineDynamicStateCreateInfo = {};
            // vkPipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            // vkPipelineDynamicStateCreateInfo.pNext = nullptr;
            // vkPipelineDynamicStateCreateInfo.flags = 0;
            // vkPipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(vkDynamicStates.size());
            // vkPipelineDynamicStateCreateInfo.pDynamicStates = vkDynamicStates.data();

            // Pipeline Rasterization State
            VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo = {};
            vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            vkPipelineRasterizationStateCreateInfo.pNext = nullptr;
            vkPipelineRasterizationStateCreateInfo.flags = 0;
            vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // Requires enabled feature "depthClamp" in logical device. When enabled it clamps fragments depth at near/far planes so they won't get clipped.
            vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // When enabled it skips rasterizer and never creates fragments.
            vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
            vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
            vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
            vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
            vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
            vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
            vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
            vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // To use value other than 1.0f it requires enable feature "wideLines" in logical device.

            // Pipeline Multisample State (MSAA)
            VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo = {};
            vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            vkPipelineMultisampleStateCreateInfo.pNext = nullptr;
            vkPipelineMultisampleStateCreateInfo.flags = 0;
            vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
            vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f;
            vkPipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
            vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
            vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

            // Pipeline Color Blend State
            //
            // Blend equation:
            // (srcColorBlendFactor * srcColor) colorBlendOp (dstColorBlendFactor * dstColor)
            // 
            // Attachment example for doing modulate blending:
            // finalColor.rgb = (srcColor.a * srcColor.rgb) + ((1 - srcColor.a) * dstColor.rgb)
            // finalColor.a   = (1 * srcColor.a) + (0 * dstColor.a)
            //
            // const VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState = {
            //     .blendEnable = VK_TRUE,
            //     .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            //     .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            //     .colorBlendOp = VK_BLEND_OP_ADD,
            //     .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            //     .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            //     .alphaBlendOp = VK_BLEND_OP_ADD,
            //     .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            // };
            //
            const VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState = {
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            };

            VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo = {};
            vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            vkPipelineColorBlendStateCreateInfo.pNext = nullptr;
            vkPipelineColorBlendStateCreateInfo.flags = 0;
            vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
            vkPipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
            vkPipelineColorBlendStateCreateInfo.attachmentCount = 1;
            vkPipelineColorBlendStateCreateInfo.pAttachments = &vkPipelineColorBlendAttachmentState;
            vkPipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            vkPipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            vkPipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            vkPipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

            // Pipeline Depth Stencil State
            VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo = {};
            vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            vkPipelineDepthStencilStateCreateInfo.pNext = nullptr;
            vkPipelineDepthStencilStateCreateInfo.flags = 0;
            vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
            vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
            vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // When true, to pass the depth test the depth value must be between minDepthBounds and maxDepthBounds
            vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f;
            vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 0.0f;
            vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
            //vkPipelineDepthStencilStateCreateInfo.front;
            //vkPipelineDepthStencilStateCreateInfo.back;

            // Finally, create the graphics pipeline
            VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
            vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            vkGraphicsPipelineCreateInfo.pNext = nullptr;
            vkGraphicsPipelineCreateInfo.flags = 0;
            vkGraphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(vkPipelineShaderStagesCreateInfo.size());
            vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStagesCreateInfo.data();
            vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pTessellationState = nullptr;
            vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pDynamicState = nullptr;
            vkGraphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
            vkGraphicsPipelineCreateInfo.renderPass = m_renderPass->GetVkRenderPass(); // Render Pass that is going to use this pipeline
            vkGraphicsPipelineCreateInfo.subpass = m_subpassIndex; // 1 pipeline can only be used in 1 subpass. Normally there are separate pipelines for each subpass.

            // For pipeline derivatives: Can create multiple pipelines that derive from one another for optimization
            vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Existing pipeline to derive from...
            vkGraphicsPipelineCreateInfo.basePipelineIndex = -1;              // or index of pipeline being created to derive from (in case creating multiple at once)

            if (vkCreateGraphicsPipelines(
                m_device->GetVkDevice(), VK_NULL_HANDLE,
                1, &vkGraphicsPipelineCreateInfo, nullptr, &m_vkPipeline) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Pipeline", "Failed to create Vulkan Pipeline.");
                return false;
            }
        }

        return true;
    }

    bool Pipeline::CreateVkPipelineLayoutSubpass1()
    {
        // TODO: Obtain this from the shaders.

        // Reset the descriptor set layouts
        m_descriptorSetLayouts.clear();

        // Descriptor Sets Layout 0: Input Attachments
        {
            const std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
                // Color Input Binding Info
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                    .descriptorCount = 1, // Number of contiguous descriptors of this type for binding in shader
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // Shader stage to bind to
                    .pImmutableSamplers = nullptr
                },
                // Depth Input Binding Info
                {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                    .descriptorCount = 1, // Number of contiguous descriptors of this type for binding in shader
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // Shader stage to bind to
                    .pImmutableSamplers = nullptr
                }
            };

            auto descriptorSetLayout = std::make_unique<DescriptorSetLayout>();

            descriptorSetLayout->m_numDynamicDescriptors = Utils::GetDynamicDescritorCount(descriptorSetLayoutBindings);

            // Create Descriptor Set Layout with given bindings
            VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo = {};
            vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            vkDescriptorSetLayoutCreateInfo.pNext = nullptr;
            vkDescriptorSetLayoutCreateInfo.flags = 0;
            vkDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
            vkDescriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

            if (vkCreateDescriptorSetLayout(m_device->GetVkDevice(),
                &vkDescriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout->m_vkDescriptorSetLayout) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Pipeline", "Failed to create Vulkan Descriptor Set Layout.");
                return false;
            }

            m_descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
        }

        // Pipeline Layout = Descriptor set layouts + Push constant ranges
        {
            std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts(m_descriptorSetLayouts.size(), nullptr);
            std::transform(m_descriptorSetLayouts.begin(), m_descriptorSetLayouts.end(), vkDescriptorSetLayouts.begin(),
                [](std::unique_ptr<DescriptorSetLayout>& descriptorSetLayout)
                {
                    return descriptorSetLayout->m_vkDescriptorSetLayout;
                });

            // Create Pipeline Layout (Resources Layout)
            // Where to specify Descriptor Set Layouts and Push Constant Ranges.
            VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo = {};
            vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            vkPipelineLayoutCreateInfo.pNext = nullptr;
            vkPipelineLayoutCreateInfo.flags = 0;
            vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
            vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();
            vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
            vkPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

            if (vkCreatePipelineLayout(m_device->GetVkDevice(), &vkPipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Pipeline", "Failed to create Vulkan Pipeline Layout.");
                return false;
            }
        }

        return true;
    }

    bool Pipeline::CreateVkPipelineSubpass1()
    {
        // Create Shader Modules
        Utils::ScopedShaderModule vertexShaderModule(m_device);
        Utils::ScopedShaderModule framentShaderModule(m_device);
        {
            const char* vertexShaderFilename = "Shaders/PostShader.vert.spv";
            const char* fragmentShaderFilename = "Shaders/PostShader.frag.spv";

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
            if (!Utils::CreateVkShaderModule(m_device->GetVkDevice(), vertexShaderByteCode.value(), &vertexShaderModule.m_vkShaderModule))
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan vertex shader module for shader %s.", vertexShaderFilename);
                return false;
            }

            if (!Utils::CreateVkShaderModule(m_device->GetVkDevice(), fragmentShaderByteCode.value(), &framentShaderModule.m_vkShaderModule))
            {
                DX_LOG(Error, "Renderer", "Failed to create Vulkan fragment shader modul for shader %s.", fragmentShaderFilename);
                return false;
            }
        }

        // Create Pipeline
        {
            // Pipeline Shader Stages
            std::array<VkPipelineShaderStageCreateInfo, 2> vkPipelineShaderStagesCreateInfo;

            vkPipelineShaderStagesCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vkPipelineShaderStagesCreateInfo[0].pNext = nullptr;
            vkPipelineShaderStagesCreateInfo[0].flags = 0;
            vkPipelineShaderStagesCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            vkPipelineShaderStagesCreateInfo[0].module = vertexShaderModule.m_vkShaderModule;
            vkPipelineShaderStagesCreateInfo[0].pName = "main";
            vkPipelineShaderStagesCreateInfo[0].pSpecializationInfo = nullptr;

            vkPipelineShaderStagesCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vkPipelineShaderStagesCreateInfo[1].pNext = nullptr;
            vkPipelineShaderStagesCreateInfo[1].flags = 0;
            vkPipelineShaderStagesCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            vkPipelineShaderStagesCreateInfo[1].module = framentShaderModule.m_vkShaderModule;
            vkPipelineShaderStagesCreateInfo[1].pName = "main";
            vkPipelineShaderStagesCreateInfo[1].pSpecializationInfo = nullptr;

            // Pipeline Vertex Input State (Input Layout) - No vertex input data for this pass, vertex positions in vertex shader.
            VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo = {};
            vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vkPipelineVertexInputStateCreateInfo.pNext = nullptr;
            vkPipelineVertexInputStateCreateInfo.flags = 0;
            vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
            vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr; // Info about data spacing, stride info
            vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
            vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr; // Info about data format and where to bind to/from

            // Pipeline Input Assembly State (Primitive Topology)
            VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo = {};
            vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            vkPipelineInputAssemblyStateCreateInfo.pNext = nullptr;
            vkPipelineInputAssemblyStateCreateInfo.flags = 0;
            vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // When true it allows overriding of "strip" topology to start new primitives

            // Viewport & Scissor State
            // Number of viewports and scissors must match in Vulkan.
            const VkViewport vkViewport = {
                .x = m_viewport.pos.x,
                .y = m_viewport.pos.y,
                .width = m_viewport.size.x,
                .height = m_viewport.size.y,
                .minDepth = 0.0f,
                .maxDepth = 1.0f
            };

            const VkRect2D vkScissor = {
                .offset = {static_cast<int32_t>(m_viewport.pos.x), static_cast<int32_t>(m_viewport.pos.y)},
                .extent = {static_cast<uint32_t>(m_viewport.size.x), static_cast<uint32_t>(m_viewport.size.y)}
            };

            VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo = {};
            vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            vkPipelineViewportStateCreateInfo.pNext = nullptr;
            vkPipelineViewportStateCreateInfo.flags = 0;
            vkPipelineViewportStateCreateInfo.viewportCount = 1;
            vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
            vkPipelineViewportStateCreateInfo.scissorCount = 1;
            vkPipelineViewportStateCreateInfo.pScissors = &vkScissor;

            // Pipeline Rasterization State
            VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo = {};
            vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            vkPipelineRasterizationStateCreateInfo.pNext = nullptr;
            vkPipelineRasterizationStateCreateInfo.flags = 0;
            vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // Requires enabled feature "depthClamp" in logical device. When enabled it clamps fragments depth at near/far planes so they won't get clipped.
            vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // When enabled it skips rasterizer and never creates fragments.
            vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
            vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
            vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
            vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
            vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
            vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
            vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
            vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // To use value other than 1.0f it requires enable feature "wideLines" in logical device.

            // Pipeline Multisample State (MSAA)
            VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo = {};
            vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            vkPipelineMultisampleStateCreateInfo.pNext = nullptr;
            vkPipelineMultisampleStateCreateInfo.flags = 0;
            vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
            vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f;
            vkPipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
            vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
            vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

            // Pipeline Color Blend State
            //
            // Blend equation:
            // (srcColorBlendFactor * srcColor) colorBlendOp (dstColorBlendFactor * dstColor)
            const VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState = {
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            };
            
            VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo = {};
            vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            vkPipelineColorBlendStateCreateInfo.pNext = nullptr;
            vkPipelineColorBlendStateCreateInfo.flags = 0;
            vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
            vkPipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
            vkPipelineColorBlendStateCreateInfo.attachmentCount = 1;
            vkPipelineColorBlendStateCreateInfo.pAttachments = &vkPipelineColorBlendAttachmentState;
            vkPipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            vkPipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            vkPipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            vkPipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

            // Pipeline Depth Stencil State - Depth test disabled
            VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo = {};
            vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            vkPipelineDepthStencilStateCreateInfo.pNext = nullptr;
            vkPipelineDepthStencilStateCreateInfo.flags = 0;
            vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
            vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
            vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
            vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // When true, to pass the depth test the depth value must be between minDepthBounds and maxDepthBounds
            vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f;
            vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 0.0f;
            vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
            //vkPipelineDepthStencilStateCreateInfo.front;
            //vkPipelineDepthStencilStateCreateInfo.back;

            // Finally, create the graphics pipeline
            VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
            vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            vkGraphicsPipelineCreateInfo.pNext = nullptr;
            vkGraphicsPipelineCreateInfo.flags = 0;
            vkGraphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(vkPipelineShaderStagesCreateInfo.size());
            vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStagesCreateInfo.data();
            vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pTessellationState = nullptr;
            vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pDynamicState = nullptr;
            vkGraphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
            vkGraphicsPipelineCreateInfo.renderPass = m_renderPass->GetVkRenderPass(); // Render Pass that is going to use this pipeline
            vkGraphicsPipelineCreateInfo.subpass = m_subpassIndex; // 1 pipeline can only be used in 1 subpass. Normally there are separate pipelines for each subpass.

            // For pipeline derivatives: Can create multiple pipelines that derive from one another for optimization
            vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Existing pipeline to derive from...
            vkGraphicsPipelineCreateInfo.basePipelineIndex = -1;              // or index of pipeline being created to derive from (in case creating multiple at once)

            if (vkCreateGraphicsPipelines(
                m_device->GetVkDevice(), VK_NULL_HANDLE,
                1, &vkGraphicsPipelineCreateInfo, nullptr, &m_vkPipeline) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Pipeline", "Failed to create Vulkan Pipeline.");
                return false;
            }
        }

        return true;
    }
} // namespace Vulkan
