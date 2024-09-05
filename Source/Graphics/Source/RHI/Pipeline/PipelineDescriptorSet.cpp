#include <RHI/Pipeline/PipelineDescriptorSet.h>

#include <RHI/Device/Device.h>
#include <RHI/Resource/Buffer/Buffer.h>
#include <RHI/Resource/Image/Image.h>
#include <RHI/Resource/ImageView/ImageView.h>
#include <RHI/Sampler/Sampler.h>
#include <RHI/Pipeline/Pipeline.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    PipelineDescriptorSet::PipelineDescriptorSet(
        Device* device, 
        VkDescriptorPool vkDescriptorPool, 
        Pipeline* pipeline,
        uint32_t setLayoutIndex)
        : m_device(device)
        , m_vkDescriptorPool(vkDescriptorPool)
        , m_pipeline(pipeline)
        , m_setLayoutIndex(setLayoutIndex)
    {
        m_descriptorSetLayout = m_pipeline->GetPipelineDescriptorSetLayout(m_setLayoutIndex);
    }

    PipelineDescriptorSet::~PipelineDescriptorSet()
    {
        Terminate();
    }

    bool PipelineDescriptorSet::Initialize()
    {
        if (m_vkDescriptorSet)
        {
            return true; // Already initialized
        }

        //DX_LOG(Info, "Vulkan PipelineDescriptorSet", "Initializing PipelineDescriptorSet...");

        if (!CreateVkDescriptorSet())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void PipelineDescriptorSet::Terminate()
    {
        //DX_LOG(Info, "Vulkan PipelineDescriptorSet", "Terminating PipelineDescriptorSet...");

        vkFreeDescriptorSets(m_device->GetVkDevice(), m_vkDescriptorPool, 1, &m_vkDescriptorSet);
        m_vkDescriptorSet = nullptr;
    }

    Pipeline* PipelineDescriptorSet::GetPipeline()
    {
        return m_pipeline;
    }

    uint32_t PipelineDescriptorSet::GetSetLayoutIndex() const
    {
        return m_setLayoutIndex;
    }

    const DescriptorSetLayout* PipelineDescriptorSet::GetDescriptorSetLayout() const
    {
        return m_descriptorSetLayout;
    }

    VkDescriptorSet PipelineDescriptorSet::GetVkDescriptorSet()
    {
        return m_vkDescriptorSet;
    }

    void PipelineDescriptorSet::SetUniformBuffer(uint32_t layoutBinding, Buffer* buffer)
    {
        // TODO: Once bindings are included in Descriptor Set Layout, this could validate
        //       that the layoutBinding is expecting a uniform buffer.

        // Descriptor for the buffer (aka a Buffer View)
        // View to the entire buffer.
        const VkDescriptorBufferInfo vkDescriptorBufferInfo = {
            .buffer = buffer->GetVkBuffer(),
            .offset = 0,
            .range = buffer->GetBufferDesc().m_elementCount * buffer->GetBufferDesc().m_elementSizeInBytes
        };

        VkWriteDescriptorSet vkWriteDescriptorSet = {};
        vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorSet.pNext = nullptr;
        vkWriteDescriptorSet.dstSet = m_vkDescriptorSet;
        // Binding index from VkDescriptorSetLayoutCreateInfo.pBindings list.
        // This is not the "binding" attribute from the shader, that's specified
        // inside each element of the list.
        vkWriteDescriptorSet.dstBinding = layoutBinding;
        vkWriteDescriptorSet.dstArrayElement = 0; // If layout binding contains an array, index in array to update.
        vkWriteDescriptorSet.descriptorCount = 1; // How many descriptors (elements in pBufferInfo) we are setting.
        vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vkWriteDescriptorSet.pImageInfo = nullptr;
        vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
        vkWriteDescriptorSet.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(m_device->GetVkDevice(), 
            1, &vkWriteDescriptorSet, 
            0, nullptr); // For copying descriptor sets to other descriptor sets
    }

    void PipelineDescriptorSet::SetUniformBufferDynamic(uint32_t layoutBinding, Buffer* buffer)
    {
        // TODO: Once bindings are included in Descriptor Set Layout, this could validate
        //       that the layoutBinding is expecting a uniform buffer dynamic.

        // Descriptor for the buffer (aka a Buffer View)
        // View of 1 element and not of the entire buffer (many objects)
        const VkDescriptorBufferInfo vkDescriptorBufferInfo = {
            .buffer = buffer->GetVkBuffer(),
            .offset = 0,
            .range = buffer->GetBufferDesc().m_elementSizeInBytes
        };

        VkWriteDescriptorSet vkWriteDescriptorSet = {};
        vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorSet.pNext = nullptr;
        vkWriteDescriptorSet.dstSet = m_vkDescriptorSet;
        // Binding index from VkDescriptorSetLayoutCreateInfo.pBindings list.
        // This is not the "binding" attribute from the shader, that's specified
        // inside each element of the list.
        vkWriteDescriptorSet.dstBinding = layoutBinding;
        vkWriteDescriptorSet.dstArrayElement = 0; // If layout binding contains an array, index in array to update.
        vkWriteDescriptorSet.descriptorCount = 1; // How many descriptors (elements in pBufferInfo) we are setting.
        vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        vkWriteDescriptorSet.pImageInfo = nullptr;
        vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
        vkWriteDescriptorSet.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(m_device->GetVkDevice(),
            1, &vkWriteDescriptorSet,
            0, nullptr); // For copying descriptor sets to other descriptor sets
    }

    void PipelineDescriptorSet::SetImageView(uint32_t layoutBinding, ImageView* imageView)
    {
        // TODO: Once bindings are included in Descriptor Set Layout, this could validate
        //       that the layoutBinding is expecting an image view.

        // Descriptor for the image (aka a Image View)
        const VkDescriptorImageInfo vkDescriptorImageInfo = {
            .sampler = nullptr,
            .imageView = imageView->GetVkImageView(),
            .imageLayout = static_cast<VkImageLayout>(imageView->GetImageDesc().m_image->GetVkImageLayout())
        };

        VkWriteDescriptorSet vkWriteDescriptorSet = {};
        vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorSet.pNext = nullptr;
        vkWriteDescriptorSet.dstSet = m_vkDescriptorSet;
        // Binding index from VkDescriptorSetLayoutCreateInfo.pBindings list.
        // This is not the "binding" attribute from the shader, that's specified
        // inside each element of the list.
        vkWriteDescriptorSet.dstBinding = layoutBinding;
        vkWriteDescriptorSet.dstArrayElement = 0; // If layout binding contains an array, index in array to update.
        vkWriteDescriptorSet.descriptorCount = 1; // How many descriptors (elements in pImageInfo) we are setting.
        vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        vkWriteDescriptorSet.pImageInfo = &vkDescriptorImageInfo;
        vkWriteDescriptorSet.pBufferInfo = nullptr;
        vkWriteDescriptorSet.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(m_device->GetVkDevice(),
            1, &vkWriteDescriptorSet,
            0, nullptr); // For copying descriptor sets to other descriptor sets
    }

    void PipelineDescriptorSet::SetSampler(uint32_t layoutBinding, Sampler* sampler)
    {
        // TODO: Once bindings are included in Descriptor Set Layout, this could validate
        //       that the layoutBinding is expecting a sampler.

        // Descriptor for the sampler
        const VkDescriptorImageInfo vkDescriptorImageInfo = {
            .sampler = sampler->GetVkSampler(),
            .imageView = nullptr,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VkWriteDescriptorSet vkWriteDescriptorSet = {};
        vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorSet.pNext = nullptr;
        vkWriteDescriptorSet.dstSet = m_vkDescriptorSet;
        // Binding index from VkDescriptorSetLayoutCreateInfo.pBindings list.
        // This is not the "binding" attribute from the shader, that's specified
        // inside each element of the list.
        vkWriteDescriptorSet.dstBinding = layoutBinding;
        vkWriteDescriptorSet.dstArrayElement = 0; // If layout binding contains an array, index in array to update.
        vkWriteDescriptorSet.descriptorCount = 1; // How many descriptors (elements in pImageInfo) we are setting.
        vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        vkWriteDescriptorSet.pImageInfo = &vkDescriptorImageInfo;
        vkWriteDescriptorSet.pBufferInfo = nullptr;
        vkWriteDescriptorSet.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(m_device->GetVkDevice(),
            1, &vkWriteDescriptorSet,
            0, nullptr); // For copying descriptor sets to other descriptor sets
    }

    bool PipelineDescriptorSet::CreateVkDescriptorSet()
    {
        VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo = {};
        vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        vkDescriptorSetAllocateInfo.pNext = nullptr;
        vkDescriptorSetAllocateInfo.descriptorPool = m_vkDescriptorPool;
        vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
        vkDescriptorSetAllocateInfo.pSetLayouts = &m_descriptorSetLayout->m_vkDescriptorSetLayout;

        if (vkAllocateDescriptorSets(
            m_device->GetVkDevice(), &vkDescriptorSetAllocateInfo, &m_vkDescriptorSet) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan PipelineDescriptorSet", "Failed to create Vulkan PipelineDescriptorSet.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
