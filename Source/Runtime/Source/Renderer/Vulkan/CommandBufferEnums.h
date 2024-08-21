#pragma once

#include <stdint.h>

namespace Vulkan
{
    // Bitwise operations on CommandBufferUsageFlag are allowed.
    enum CommandBufferUsageFlag
    {
        CommandBufferUsage_OneTimeSubmit = 1 << 0, // Command Buffer will become invalid after 1 submit
        CommandBufferUsage_RenderPassContinue = 1 << 1,
        CommandBufferUsage_SimultaneousUse = 1 << 2 // Command Buffer can be resubmitted to the queue when it has already been submitted and is awaiting execution or executing.
    };
    using CommandBufferUsageFlags = uint32_t;

} // namespace Vulkan
