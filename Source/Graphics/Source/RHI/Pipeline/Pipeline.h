#pragma once

#include <RHI/Resource/ResourceEnums.h>

#include <Math/Rectangle.h>

#include <vector>

typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;

namespace Vulkan
{
    class Device;
    class RenderPass;
    class PipelineDescriptorSet;

    constexpr int PushConstantsMaxSize = 128; // Bytes

    struct DescriptorSetLayout
    {
        VkDescriptorSetLayout m_vkDescriptorSetLayout = nullptr;
        uint32_t m_numDynamicDescriptors = 0;
    };

    // Manages the Vulkan graphics pipeline
    class Pipeline
    {
    public:
        Pipeline(Device* device, RenderPass* renderPass, uint32_t subpassIndex, const Math::Rectangle& viewport);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        bool Initialize();
        void Terminate();

        RenderPass* GetRenderPass();
        uint32_t GetSubpassIndex() const;

        VkPipeline GetVkPipeline();
        VkPipelineLayout GetVkPipelineLayout();
        DescriptorSetLayout* GetPipelineDescriptorSetLayout(uint32_t setLayoutIndex);

        // The object returned has the layout necessary from the shaders of this pipeline.
        // It'll have the right number of descriptors, but the descriptors will have to
        // be filled (bound with resources) before using it in CommandBuffer::BindPipelineDescriptorSet.
        // The pipeline is not responsible for filling the resources or destroying the descriptor set.
        // 
        // Since resources are bound at descriptor set level, it'd be more optimal to group resources
        // that are updated with the same frequency. For example, use one descriptor set for per scene
        // resources, other for per material resources and so on.
        std::shared_ptr<PipelineDescriptorSet> CreatePipelineDescriptorSet(uint32_t setLayoutIndex);

    private:
        Device* m_device = nullptr;
        RenderPass* m_renderPass = nullptr;
        uint32_t m_subpassIndex = std::numeric_limits<uint32_t>::max();
        Math::Rectangle m_viewport;

    private:
        // TODO: At the moment the pipelines are manually created since the user cannot specify
        //       from a PipelineDesc the parameters. Refactor after there is a PipelineDesc struct.
        bool CreateVkPipelineLayoutSubpass0();
        bool CreateVkPipelineSubpass0();
        bool CreateVkPipelineLayoutSubpass1();
        bool CreateVkPipelineSubpass1();

        std::vector<std::unique_ptr<DescriptorSetLayout>> m_descriptorSetLayouts;
        VkPipelineLayout m_vkPipelineLayout = nullptr;

        VkPipeline m_vkPipeline = nullptr;
    };
} // namespace Vulkan
