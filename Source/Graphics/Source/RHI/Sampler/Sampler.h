#pragma once

#include <RHI/Sampler/SamplerDesc.h>

typedef struct VkSampler_T* VkSampler;

namespace Vulkan
{
    class Device;

    class Sampler
    {
    public:
        Sampler(Device* device, const SamplerDesc& desc);
        ~Sampler();

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;

        bool Initialize();
        void Terminate();

        VkSampler GetVkSampler();

    private:
        Device* m_device = nullptr;
        SamplerDesc m_desc;

    private:
        bool CreateVkSampler();

        VkSampler m_vkSampler = nullptr;
    };
} // namespace Vulkan
