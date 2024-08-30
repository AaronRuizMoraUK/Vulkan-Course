#include <Renderer/Vulkan/Pipeline.h>

#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/PipelineDescriptorSet.h>

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

    Pipeline::Pipeline(Device* device, int imageFormat, const Math::Rectangle& viewport)
        : m_device(device)
        , m_imageFormat(imageFormat)
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

        if (!CreateVkRenderPass())
        {
            Terminate();
            return false;
        }

        if (!CreateVkPipelineLayout())
        {
            Terminate();
            return false;
        }

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

        vkDestroyPipeline(m_device->GetVkDevice(), m_vkPipeline, nullptr);
        m_vkPipeline = nullptr;

        vkDestroyPipelineLayout(m_device->GetVkDevice(), m_vkPipelineLayout, nullptr);
        m_vkPipelineLayout = nullptr;

        std::ranges::for_each(m_descriptorSetLayouts, [this](std::unique_ptr<DescriptorSetLayout>& descriptorSetLayout)
            {
                vkDestroyDescriptorSetLayout(m_device->GetVkDevice(), descriptorSetLayout->m_vkDescriptorSetLayout, nullptr);
            });
        m_descriptorSetLayouts.clear();

        vkDestroyRenderPass(m_device->GetVkDevice(), m_vkRenderPass, nullptr);
        m_vkRenderPass = nullptr;
    }

    VkRenderPass Pipeline::GetVkRenderPass()
    {
        return m_vkRenderPass;
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
            return std::make_shared<PipelineDescriptorSet>(
                m_device, m_device->GetVkDescriptorPool(), this, setLayoutIndex);
        }
        else
        {
            return nullptr;
        }
    }

    bool Pipeline::CreateVkRenderPass()
    {
        // About image layouts in attachments
        // 
        // Frame buffer data will be stored as an image, but images can be given
        // different data layouts to give optimal use for certain operations (read, write, present, etc).
        //
        // The layouts are specified in the attachments assigned to render passes and subpasses,
        // and they indicate the layout the image has to be and the layout the image has changed to 
        // when the passes and subpasses are being executed.
        //
        // In our responsibility to specify the correct image layouts for the images while they
        // are being used by render passes and subpasses.

        // Color attachment of the render pass
        const VkAttachmentDescription colorAttachment = {
            .flags = 0,
            .format = static_cast<VkFormat>(m_imageFormat), // Format to use for attachment
            .samples = VK_SAMPLE_COUNT_1_BIT, // Number of samples to write for MSAA

            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // What to do with attachment before rendering
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // What to do with attachment after rendering
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // Image data layout expected before render pass starts
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // Image data layout to convert to after render pass finishes
        };

        // Render subpass
        // 
        // A subpass has references to Render Pass's attachment descriptors (vkRenderPassCreateInfo.pAttachments),
        // not the attachment descriptors themselves. The reference is specify with an Attachment Reference, where
        // indices to vkRenderPassCreateInfo.pAttachments are specified.
        const VkAttachmentReference colorAttachmentReference = {
            .attachment = 0, // Index of the attachment inside vkRenderPassCreateInfo.pAttachments
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // Image data layout to convert to before render subpass starts
        };

        const VkSubpassDescription subpass = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, // What pipeline the subpass is to be bound to
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentReference,
            .pResolveAttachments = 0,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        // Subpass dependency
        // 
        // A subpass dependency is needed to determine when layout transitions occur.
        // It does implicit layout transitions, the attachments have WHAT layouts
        // to have, but with the subpass dependency we indicate WHEN we want the
        // layout transition operation to occur.
        //
        // It's our responsibility to specify the right points within the render pass when the layout
        // transition can start and when it needs to be finished. For example, after subpass A
        // finishes you can start the transition and be done before this subpass B starts.
        // Notice we haven't say explicitly when the operation happens, but indicated a range in time
        // when the GPU will need to do the operation.
        //
        // In the subpass dependency we specify not only between which subpasses the operation need
        // to happen, but we also specify at what stage inside the subpass' pipeline can the operation start
        // and expected to finish. For example, start after Vertex Shader of subpass A and finish before
        // Fragment Shader of subpass B.
        //
        // Special keywords to be aware of:
        // - Subpass index VK_SUBPASS_EXTERNAL: It means "anything that takes place outside our subpasses".
        // - Stage Mask VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT: It means "at the end of the subpass' pipeline".
        //
        // Finally, there is another level (beyond stage) where we can specify when operation needs to start/finish,
        // that's the Access Mask, basically means before/after "what operation within the stage".
        // The following website lists all the Access Mask values allowed and in what stages they can be used:
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html
        //
        std::array<VkSubpassDependency, 2> subpassDependencies;

        // Layout Conversion 1: VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        // Start after: End of whatever came before
        // Finish before: Color Output stage in subpass 0
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; // Conversion has to start after: it has to be read from
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Conversion has to finish before: reading or writing to it
        subpassDependencies[0].dependencyFlags = 0;

        // Layout Conversion 2: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        // Start after: Color Output stage in subpass 0
        // Finish before: Whatever comes after tries to read from it
        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Conversion has to start after: reading or writing to it
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; // Conversion has to finish before: it has to be read from
        subpassDependencies[1].dependencyFlags = 0;

        // -----------
        // Render Pass
        // 
        // This is the render pass we're building:
        //
        // RENDER PASS
        //      Color Attachment initial layout: VK_IMAGE_LAYOUT_UNDEFINED
        //
        //      Subpass dependency 1: Convert VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        // 
        //      SUBPASS 1
        //          Color Attachment layout: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        //          Draws to color attachment
        //
        //      Subpass dependency 2: Convert VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        //
        //      Color Attachment final layout: VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        //      Present color attachment to surface
        //
        VkRenderPassCreateInfo vkRenderPassCreateInfo = {};
        vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        vkRenderPassCreateInfo.pNext = nullptr;
        vkRenderPassCreateInfo.flags = 0;
        vkRenderPassCreateInfo.attachmentCount = 1;
        vkRenderPassCreateInfo.pAttachments = &colorAttachment;
        vkRenderPassCreateInfo.subpassCount = 1;
        vkRenderPassCreateInfo.pSubpasses = &subpass;
        vkRenderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        vkRenderPassCreateInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(m_device->GetVkDevice(), &vkRenderPassCreateInfo, nullptr, &m_vkRenderPass) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan Pipeline", "Failed to create Vulkan Render Pass.");
            return false;
        }

        return true;
    }

    bool Pipeline::CreateVkPipelineLayout()
    {
        // Descriptor Sets Layout
        // TODO: Obtain this from the shaders.
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

            m_descriptorSetLayouts.clear();
            m_descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
        }

        // Push Constant Ranges
        // TODO: Obtain this from the shaders.
        const std::vector<VkPushConstantRange> vkPushConstantRanges = {
            // World Binding Info
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = 2 * sizeof(Math::Matrix4x4Packed) // Max size 128 bytes. It fits 2 matrices.
            }
        };

        // Pipeline Layout
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

    bool Pipeline::CreateVkPipeline()
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
        ScopedShaderModule vertexShaderModule(m_device);
        ScopedShaderModule framentShaderModule(m_device);
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
                .stride = (3 + 4) * 4, // sizeof(VertexPC)
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            };

            const std::vector<VkVertexInputAttributeDescription> vkVertexInputAttributesDesc = {
                {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},
                {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 3 * 4 /*offsetof(VertexPC, m_color)*/}
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

            // TODO: Pipeline Depth Stencil State

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
            vkGraphicsPipelineCreateInfo.pDepthStencilState = nullptr;
            vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
            vkGraphicsPipelineCreateInfo.pDynamicState = nullptr;
            vkGraphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
            vkGraphicsPipelineCreateInfo.renderPass = m_vkRenderPass; // Render Pass that is going to use this pipeline
            vkGraphicsPipelineCreateInfo.subpass = 0; // 1 pipeline can only be used in 1 subpass. Normally there are separate pipelines for each subpass.

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
