#include <Renderer/Vulkan/PipelineDescriptorSet.h>

#include <Renderer/Vulkan/Device.h>
#include <Renderer/Vulkan/Buffer.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    PipelineDescriptorSet::PipelineDescriptorSet(
        Device* device, 
        VkDescriptorPool vkDescriptorPool, 
        VkDescriptorSetLayout vkDescriptorSetLayout,
        VkPipelineLayout vkPipelineLayout)
        : m_device(device)
        , m_vkDescriptorPool(vkDescriptorPool)
        , m_vkDescriptorSetLayout(vkDescriptorSetLayout)
        , m_vkPipelineLayout(vkPipelineLayout)
    {
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

        DX_LOG(Info, "Vulkan PipelineDescriptorSet", "Initializing PipelineDescriptorSet...");

        if (!CreateVkDescriptorSet())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void PipelineDescriptorSet::Terminate()
    {
        DX_LOG(Info, "Vulkan PipelineDescriptorSet", "Terminating PipelineDescriptorSet...");

        vkFreeDescriptorSets(m_device->GetVkDevice(), m_vkDescriptorPool, 1, &m_vkDescriptorSet);
        m_vkDescriptorSet = nullptr;
    }

    VkDescriptorSet PipelineDescriptorSet::GetVkDescriptorSet()
    {
        return m_vkDescriptorSet;
    }

    VkPipelineLayout PipelineDescriptorSet::GetVkPipelineLayout()
    {
        return m_vkPipelineLayout;
    }

    void PipelineDescriptorSet::SetUniformBuffer(uint32_t layoutBinding, Buffer* buffer)
    {
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

    bool PipelineDescriptorSet::CreateVkDescriptorSet()
    {
        VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo = {};
        vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        vkDescriptorSetAllocateInfo.pNext = nullptr;
        vkDescriptorSetAllocateInfo.descriptorPool = m_vkDescriptorPool;
        vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
        vkDescriptorSetAllocateInfo.pSetLayouts = &m_vkDescriptorSetLayout;

        if (vkAllocateDescriptorSets(
            m_device->GetVkDevice(), &vkDescriptorSetAllocateInfo, &m_vkDescriptorSet) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan PipelineDescriptorSet", "Failed to create Vulkan PipelineDescriptorSet.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
