#include <Renderer/Vulkan/Utils.h>

namespace Vulkan
{

    VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags flags)
    {
        VkBufferUsageFlags vkBufferUsageFlags = 0;

        vkBufferUsageFlags |= (flags & BufferUsage_VertexBuffer) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
        vkBufferUsageFlags |= (flags & BufferUsage_IndexBuffer) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
        vkBufferUsageFlags |= (flags & BufferUsage_UniformBuffer) ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;

        return vkBufferUsageFlags;
    }

    VkCommandBufferUsageFlags ToVkCommandBufferUsageFlags(CommandBufferUsageFlags flags)
    {
        VkCommandBufferUsageFlags vkCommandBufferUsageFlags = 0;

        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_OneTimeSubmit) ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_RenderPassContinue) ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0;
        vkCommandBufferUsageFlags |= (flags & CommandBufferUsage_SimultaneousUse) ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0;

        return vkCommandBufferUsageFlags;
    }

    VkShaderStageFlags ToVkShaderStageFlags(ShaderType shaderType)
    {
        switch (shaderType)
        {
        case ShaderType_Vertex:                return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderType_TesselationControl:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderType_TesselationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderType_Geometry:              return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderType_Fragment:              return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderType_Compute:               return VK_SHADER_STAGE_COMPUTE_BIT;

        case ShaderType_Unknown:
        default:
            return 0;
        }
    }

} // namespace Vulkan
