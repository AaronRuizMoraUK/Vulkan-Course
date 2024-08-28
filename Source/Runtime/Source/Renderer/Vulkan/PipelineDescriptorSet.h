#pragma once

#include <stdint.h>
#include <limits>

typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef struct VkPipelineLayout_T* VkPipelineLayout;

namespace Vulkan
{
    class Device;
    class Buffer;

    // Manages a Pipeline Descriptor Set
    class PipelineDescriptorSet
    {
    public:
        PipelineDescriptorSet(
            Device* device, 
            VkDescriptorPool vkDescriptorPool, 
            VkDescriptorSetLayout vkDescriptorSetLayout,
            VkPipelineLayout vkPipelineLayout,
            uint32_t setLayoutIndex);
        ~PipelineDescriptorSet();

        PipelineDescriptorSet(const PipelineDescriptorSet&) = delete;
        PipelineDescriptorSet& operator=(const PipelineDescriptorSet&) = delete;

        bool Initialize();
        void Terminate();

        VkDescriptorSet GetVkDescriptorSet();
        VkPipelineLayout GetVkPipelineLayout();
        uint32_t GetSetLayoutIndex() const;

        void SetUniformBuffer(uint32_t layoutBinding, Buffer* buffer);

    private:
        Device* m_device = nullptr;
        VkDescriptorPool m_vkDescriptorPool = nullptr;
        VkDescriptorSetLayout m_vkDescriptorSetLayout = nullptr;
        VkPipelineLayout m_vkPipelineLayout = nullptr;
        uint32_t m_setLayoutIndex = std::numeric_limits<uint32_t>::max(); // Index of this descriptor set layout inside the pipeline layout

    private:
        bool CreateVkDescriptorSet();

        VkDescriptorSet m_vkDescriptorSet = nullptr;
    };
} // namespace Vulkan
