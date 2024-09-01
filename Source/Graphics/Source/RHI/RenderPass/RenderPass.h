#pragma once

#include <RHI/Resource/ResourceEnums.h>

#include <vector>

typedef struct VkRenderPass_T* VkRenderPass;

namespace Vulkan
{
    class Device;

    // Manages the Vulkan Render Pass
    class RenderPass
    {
    public:
        RenderPass(Device* device, ResourceFormat colorFormat, ResourceFormat depthStencilFormat);
        ~RenderPass();

        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        bool Initialize();
        void Terminate();

        VkRenderPass GetVkRenderPass();

    private:
        Device* m_device = nullptr;
        ResourceFormat m_colorFormat = ResourceFormat::Unknown;
        ResourceFormat m_depthStencilFormat = ResourceFormat::Unknown;

    private:
        bool CreateVkRenderPass();

        VkRenderPass m_vkRenderPass = nullptr;
    };
} // namespace Vulkan
