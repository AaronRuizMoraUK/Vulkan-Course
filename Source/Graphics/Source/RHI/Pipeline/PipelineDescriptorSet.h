#pragma once

#include <stdint.h>
#include <limits>

typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkDescriptorPool_T* VkDescriptorPool;

namespace Vulkan
{
    class Device;
    class Pipeline;
    class Buffer;
    class ImageView;
    class Sampler;
    struct DescriptorSetLayout;

    // Manages a Pipeline Descriptor Set
    class PipelineDescriptorSet
    {
    public:
        PipelineDescriptorSet(
            Device* device, 
            VkDescriptorPool vkDescriptorPool, 
            Pipeline* pipeline,
            uint32_t setLayoutIndex);
        ~PipelineDescriptorSet();

        PipelineDescriptorSet(const PipelineDescriptorSet&) = delete;
        PipelineDescriptorSet& operator=(const PipelineDescriptorSet&) = delete;

        bool Initialize();
        void Terminate();

        Pipeline* GetPipeline();
        uint32_t GetSetLayoutIndex() const;
        const DescriptorSetLayout* GetDescriptorSetLayout() const;
        VkDescriptorSet GetVkDescriptorSet();

        // Set shader resources using layout binding index inside the descriptor set layout.
        void SetShaderUniformBuffer(uint32_t layoutBinding, Buffer* buffer);
        void SetShaderUniformBufferDynamic(uint32_t layoutBinding, Buffer* buffer);
        void SetShaderSampledImageView(uint32_t layoutBinding, ImageView* imageView);
        void SetShaderSampler(uint32_t layoutBinding, Sampler* sampler);
        void SetShaderInputAttachment(uint32_t layoutBinding, ImageView* imageView);

    private:
        Device* m_device = nullptr;
        VkDescriptorPool m_vkDescriptorPool = nullptr;
        Pipeline* m_pipeline = nullptr;
        uint32_t m_setLayoutIndex = std::numeric_limits<uint32_t>::max(); // Index of this descriptor set layout inside the pipeline layout
        const DescriptorSetLayout* m_descriptorSetLayout = nullptr;

    private:
        bool CreateVkDescriptorSet();

        VkDescriptorSet m_vkDescriptorSet = nullptr;
    };
} // namespace Vulkan
