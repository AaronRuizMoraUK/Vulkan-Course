#pragma once

#include <Math/Rectangle.h>

typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkPipeline_T* VkPipeline;

namespace Vulkan
{
    class Device;

    // Manages the Vulkan pipeline
    class Pipeline
    {
    public:
        Pipeline(Device* device, int imageFormat, const Math::Rectangle& viewport);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        bool Initialize();
        void Terminate();

    private:
        Device* m_device = nullptr;
        int m_imageFormat = -1;
        Math::Rectangle m_viewport;

    private:
        bool CreateVkRenderPass();
        bool CreateVkPipelineLayout();
        bool CreateVkPipeline();

        VkRenderPass m_vkRenderPass = nullptr;
        VkPipelineLayout m_vkPipelineLayout = nullptr;
        VkPipeline m_vkPipeline = nullptr;
    };
} // namespace Vulkan
