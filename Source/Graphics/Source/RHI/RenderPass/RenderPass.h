#pragma once

#include <RHI/RenderPass/RenderPassDesc.h>

#include <vector>

typedef struct VkRenderPass_T* VkRenderPass;

namespace Vulkan
{
    class Device;

    // Manages the Vulkan Render Pass
    class RenderPass
    {
    public:
        RenderPass(Device* device, const RenderPassDesc& desc);
        ~RenderPass();

        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        bool Initialize();
        void Terminate();

        VkRenderPass GetVkRenderPass();

    private:
        Device* m_device = nullptr;
        RenderPassDesc m_desc;

    private:
        bool CreateVkRenderPass();

        VkRenderPass m_vkRenderPass = nullptr;
    };
} // namespace Vulkan
