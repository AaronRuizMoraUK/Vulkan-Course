#pragma once

namespace Vulkan
{
    class Device;

    // Manages the Vulkan pipeline
    class Pipeline
    {
    public:
        Pipeline(Device* device);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        bool Initialize();
        void Terminate();

    private:
        Device* m_device = nullptr;

    private:
        bool CreateVkPipeline();
    };
} // namespace Vulkan
