#pragma once

#include <stdint.h>

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
            VkPipelineLayout vkPipelineLayout);
        ~PipelineDescriptorSet();

        PipelineDescriptorSet(const PipelineDescriptorSet&) = delete;
        PipelineDescriptorSet& operator=(const PipelineDescriptorSet&) = delete;

        bool Initialize();
        void Terminate();

        VkDescriptorSet GetVkDescriptorSet();
        VkPipelineLayout GetVkPipelineLayout();

        void SetUniformBuffer(uint32_t layoutBinding, Buffer* buffer);

    private:
        Device* m_device = nullptr;
        VkDescriptorPool m_vkDescriptorPool = nullptr;
        VkDescriptorSetLayout m_vkDescriptorSetLayout = nullptr;
        VkPipelineLayout m_vkPipelineLayout = nullptr;

    private:
        bool CreateVkDescriptorSet();

        VkDescriptorSet m_vkDescriptorSet = nullptr;
    };
} // namespace Vulkan
