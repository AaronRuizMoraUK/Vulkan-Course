#pragma once

#include <Math/Rectangle.h>

typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;

namespace Vulkan
{
    class Device;
    class PipelineDescriptorSet;

    // Manages the Vulkan graphics pipeline
    class Pipeline
    {
    public:
        Pipeline(Device* device, int imageFormat, const Math::Rectangle& viewport);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        bool Initialize();
        void Terminate();

        VkRenderPass GetVkRenderPass();
        VkPipeline GetVkPipeline();

        // The object returned has the layout necessary from the shaders of this pipeline.
        // It'll have the right number of descriptors, but the descriptors will have to
        // be filled (bound with resources) before using it in CommandBuffer::BindPipelineDescriptorSet.
        // The pipeline is not responsible for filling the resources or destroying the descriptor set.
        // 
        // Since resources are bound at descriptor set level, it'd be more optimal to group resources
        // that are updated with the same frequency. For example, use one descriptor set for per scene
        // resources, other for per material resources and other for per object resources.
        //
        // TODO: This is only a part of the pipeline layout. Ideally this should be CreatePipelineLayoutObject,
        //       which will know how to set resources into the several descriptor sets (one per shader) or push 
        //       constants it has inside. This at the moment assumes Pipeline Layout has only 1 descriptor set
        //       and no push constants.
        std::shared_ptr<PipelineDescriptorSet> CreatePipelineDescriptorSet() const;

    private:
        Device* m_device = nullptr;
        int m_imageFormat = -1;
        Math::Rectangle m_viewport;

    private:
        bool CreateVkRenderPass();
        bool CreateVkDescriptorSetLayout();
        bool CreateVkPipelineLayout();
        bool CreateVkPipeline();

        // TODO: move it to its own class and pass it to Pipeline instead of imageFormat
        VkRenderPass m_vkRenderPass = nullptr;

        // TODO: Obtain this from the shaders.
        VkDescriptorSetLayout m_vkDescriptorSetLayout = nullptr;
        VkPipelineLayout m_vkPipelineLayout = nullptr;

        VkPipeline m_vkPipeline = nullptr;
    };
} // namespace Vulkan
