#include <RHI/Sampler/Sampler.h>

#include <RHI/Device/Device.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    Sampler::Sampler(Device* device, const SamplerDesc& desc)
        : m_device(device)
        , m_desc(desc)
    {
    }

    Sampler::~Sampler()
    {
        Terminate();
    }

    bool Sampler::Initialize()
    {
        if (m_vkSampler)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan Sampler", "Initializing Vulkan Sampler...");

        if (!CreateVkSampler())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Sampler::Terminate()
    {
        DX_LOG(Info, "Vulkan Sampler", "Terminating Vulkan Sampler...");

        vkDestroySampler(m_device->GetVkDevice(), m_vkSampler, nullptr);
        m_vkSampler = nullptr;
    }

    VkSampler Sampler::GetVkSampler()
    {
        return m_vkSampler;
    }

    bool Sampler::CreateVkSampler()
    {
        VkSamplerCreateInfo vkSamplerCreateInfo = {};
        vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        vkSamplerCreateInfo.pNext = nullptr;
        vkSamplerCreateInfo.flags = 0;
        vkSamplerCreateInfo.magFilter = ToVkFilter(m_desc.m_magFilter);
        vkSamplerCreateInfo.minFilter = ToVkFilter(m_desc.m_minFilter);
        vkSamplerCreateInfo.mipmapMode = ToVkSamplerMipmapMode(m_desc.m_mipFilter);
        vkSamplerCreateInfo.addressModeU = ToVkSamplerAddressMode(m_desc.m_addressU);
        vkSamplerCreateInfo.addressModeV = ToVkSamplerAddressMode(m_desc.m_addressV);
        vkSamplerCreateInfo.addressModeW = ToVkSamplerAddressMode(m_desc.m_addressW);
        vkSamplerCreateInfo.mipLodBias = m_desc.m_mipBias;
        vkSamplerCreateInfo.anisotropyEnable =
            m_desc.m_magFilter == FilterSampling::Anisotropic ||
            m_desc.m_minFilter == FilterSampling::Anisotropic ||
            m_desc.m_mipFilter == FilterSampling::Anisotropic;
        vkSamplerCreateInfo.maxAnisotropy = m_desc.m_maxAnisotropy;
        vkSamplerCreateInfo.compareEnable = VK_FALSE;
        vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        vkSamplerCreateInfo.minLod = m_desc.m_mipClamp.x;
        vkSamplerCreateInfo.maxLod = m_desc.m_mipClamp.y;
        vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // False means it is using normalized coordinates (between 0 and 1)

        if (vkCreateSampler(m_device->GetVkDevice(), &vkSamplerCreateInfo, nullptr, &m_vkSampler) != VK_SUCCESS)
        {
            DX_LOG(Error, "Vulkan Sampler", "Failed to create Vulkan Sampler.");
            return false;
        }

        return true;
    }
} // namespace Vulkan
