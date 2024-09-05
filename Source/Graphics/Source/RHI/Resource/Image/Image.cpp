#include <RHI/Resource/Image/Image.h>

#include <RHI/Device/Device.h>
#include <RHI/CommandBuffer/CommandBuffer.h>
#include <RHI/Resource/Buffer/Buffer.h>
#include <RHI/Vulkan/Utils.h>

#include <Log/Log.h>
#include <Debug/Debug.h>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    namespace Utils
    {
        bool CreateVkImage(
            Device* device,
            VkImageType vkImageType,
            const Math::Vector3Int& dimensions,
            uint32_t mipCount,
            VkFormat vkFormat,
            VkImageTiling vkImageTiling,
            VkImageUsageFlags vkImageUsageFlags,
            VkMemoryPropertyFlags vkMemoryPropertyFlags,
            VkImage* vkImageOut,
            VkDeviceMemory* vkImageMemoryOut)
        {
            // Create Image object
            {
                // If queue families use different queues, then image must be shared between families.
                const std::vector<uint32_t>& uniqueFamilyIndices = device->GetQueueFamilyInfo().m_uniqueQueueFamilyIndices;

                VkImageCreateInfo vkImageCreateInfo = {};
                vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                vkImageCreateInfo.pNext = nullptr;
                vkImageCreateInfo.flags = 0;
                vkImageCreateInfo.imageType = vkImageType;
                vkImageCreateInfo.format = vkFormat;
                vkImageCreateInfo.extent.width = dimensions.x;
                vkImageCreateInfo.extent.height = dimensions.y;
                vkImageCreateInfo.extent.depth = dimensions.z; // Must be greater than 0
                vkImageCreateInfo.mipLevels = mipCount;
                vkImageCreateInfo.arrayLayers = 1;
                vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT; // Number of samples for multi-sampling
                vkImageCreateInfo.tiling = vkImageTiling;
                vkImageCreateInfo.usage = vkImageUsageFlags;
                if (uniqueFamilyIndices.size() > 1)
                {
                    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
                    vkImageCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(uniqueFamilyIndices.size());
                    vkImageCreateInfo.pQueueFamilyIndices = uniqueFamilyIndices.data();
                }
                else
                {
                    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    vkImageCreateInfo.queueFamilyIndexCount = 0;
                    vkImageCreateInfo.pQueueFamilyIndices = nullptr;
                }
                vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Layout of image data on creation

                if (vkCreateImage(device->GetVkDevice(), &vkImageCreateInfo, nullptr, vkImageOut) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Image", "Failed to create Vulkan Image.");
                    return false;
                }
            }

            // Allocate memory for image and link them together
            {
                // Get Image's Memory Requirements
                VkMemoryRequirements vkMemoryRequirements = {};
                vkGetImageMemoryRequirements(device->GetVkDevice(), *vkImageOut, &vkMemoryRequirements);

                // Allocate memory for the image
                VkMemoryAllocateInfo vkMemoryAllocateInfo = {};
                vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                vkMemoryAllocateInfo.pNext = nullptr;
                vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
                vkMemoryAllocateInfo.memoryTypeIndex = FindCompatibleMemoryTypeIndex(
                    device->GetVkPhysicalDevice(), vkMemoryRequirements.memoryTypeBits, vkMemoryPropertyFlags);

                if (vkAllocateMemory(device->GetVkDevice(), &vkMemoryAllocateInfo, nullptr, vkImageMemoryOut) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Image", "Failed to allocate memory for Vulkan Image.");
                    return false;
                }

                // Link the image to the memory
                if (vkBindImageMemory(device->GetVkDevice(), *vkImageOut, *vkImageMemoryOut, 0) != VK_SUCCESS)
                {
                    DX_LOG(Error, "Vulkan Image", "Failed to bind Vulkan image to memory.");
                    return false;
                }
            }

            return true;
        }

        void DestroyVkImage(Device* device, VkImage& vkImage, VkDeviceMemory& vkImageMemory)
        {
            vkDestroyImage(device->GetVkDevice(), vkImage, nullptr);
            vkImage = nullptr;

            vkFreeMemory(device->GetVkDevice(), vkImageMemory, nullptr);
            vkImageMemory = nullptr;
        }

        bool ExecuteCommandBufferAndWait(Device* device, CommandBuffer* commandBuffer)
        {
            VkCommandBuffer vkCommandBuffer = commandBuffer->GetVkCommandBuffer();

            VkSubmitInfo vkSubmitInfo = {};
            vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            vkSubmitInfo.pNext = nullptr;
            vkSubmitInfo.pWaitDstStageMask = nullptr;
            vkSubmitInfo.waitSemaphoreCount = 0;
            vkSubmitInfo.pWaitSemaphores = nullptr;
            vkSubmitInfo.commandBufferCount = 1;
            vkSubmitInfo.pCommandBuffers = &vkCommandBuffer;
            vkSubmitInfo.signalSemaphoreCount = 0;
            vkSubmitInfo.pSignalSemaphores = nullptr;

            // Submit command buffer to queue for execution by the GPU
            if (vkQueueSubmit(device->GetVkQueue(Vulkan::QueueFamilyType_Graphics),
                1, &vkSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
            {
                DX_LOG(Error, "Vulkan Buffer", "Failed to submit transfer work to the queue.");
                return false;
            }

            // Wait until the queue has finished executing all its commands.
            // 
            // NOTE: This is a sequential way to transfer images. More advance techniques
            //       would not block and allow to continue adding transfer commands for other
            //       images and synchronize at some point in the future.
            vkQueueWaitIdle(device->GetVkQueue(Vulkan::QueueFamilyType_Graphics));

            return true;
        }

        bool CopyBufferToImage(Device* device, Image* dstImage, Buffer* srcBuffer)
        {
            // Command buffer for transfer commands.
            // By Vulkan standards, graphical queues also support transfer commands.
            CommandBuffer transferCmdBuffer(device,
                device->GetVkCommandPool(QueueFamilyType_Graphics, ResourceTransferCommandPoolIndex));
            if (transferCmdBuffer.Initialize())
            {
                // Record transfer commands to the command buffer.
                // Since it's a transfer operation, there is no need to bind render pass or pipelines.
                // NOTE: To be more accurate, there is an implicit pipeline of only 1 stage which is to do the transfer.
                if (transferCmdBuffer.Begin(CommandBufferUsage_OneTimeSubmit))
                {
                    transferCmdBuffer.CopyBufferToImage(dstImage, srcBuffer);

                    transferCmdBuffer.End();
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            // Execute the transfer command buffer
            if (!ExecuteCommandBufferAndWait(device, &transferCmdBuffer))
            {
                return false;
            }

            return true;
        }

        bool TransitionImageLayout(Device* device, Image* image, 
            VkImageLayout vkOldImageLayout, VkImageLayout vkNewImageLayout,
            VkPipelineStageFlags vkSrcPipelineStage, VkAccessFlags vkSrcAccessMask,
            VkPipelineStageFlags vkDstPipelineStage, VkAccessFlags vkDstAccessMask)
        {
            CommandBuffer cmdBuffer(device,
                device->GetVkCommandPool(QueueFamilyType_Graphics, ResourceTransferCommandPoolIndex));
            if (cmdBuffer.Initialize())
            {
                // Record commands to the command buffer.
                // Since it's a transfer operation, there is no need to bind render pass or pipelines.
                // NOTE: To be more accurate, there is an implicit pipeline of only 1 stage which is to do the operation.
                if (cmdBuffer.Begin(CommandBufferUsage_OneTimeSubmit))
                {
                    cmdBuffer.PipelineImageMemoryBarrier(image, 
                        vkOldImageLayout, vkNewImageLayout,
                        vkSrcPipelineStage, vkSrcAccessMask,
                        vkDstPipelineStage, vkDstAccessMask);

                    cmdBuffer.End();
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            // Execute the command buffer
            if (!ExecuteCommandBufferAndWait(device, &cmdBuffer))
            {
                return false;
            }

            return true;
        }
    } // Utils

    Image::Image(Device* device, const ImageDesc& desc)
        : m_device(device)
        , m_desc(desc)
    {
    }

    Image::~Image()
    {
        Terminate();
    }

    bool Image::Initialize()
    {
        if (m_vkImage)
        {
            return true; // Already initialized
        }

        DX_LOG(Info, "Vulkan Image", "Initializing Vulkan Image...");

        if (!CreateVkImage())
        {
            Terminate();
            return false;
        }

        return true;
    }

    void Image::Terminate()
    {
        DX_LOG(Info, "Vulkan Image", "Terminating Vulkan Image...");

        if (m_vkImage && !m_desc.m_nativeResource.has_value())
        {
            DX_LOG(Verbose, "Vulkan Image", "Image %s %dx%dx%d and %d mipmaps destroyed.",
                ImageTypeStr(m_desc.m_imageType), m_desc.m_dimensions.x, m_desc.m_dimensions.y, m_desc.m_dimensions.z, m_desc.m_mipCount);
        }

        const bool skipDestruction = 
            m_desc.m_nativeResource.has_value() && 
            !m_desc.m_nativeResource->m_ownsNativeResource;

        if (skipDestruction)
        {
            m_vkImage = nullptr;
            m_vkImageMemory = nullptr;
        }
        else
        {
            Utils::DestroyVkImage(m_device, m_vkImage, m_vkImageMemory);
        }
    }

    VkImage Image::GetVkImage()
    {
        return m_vkImage;
    }

    int Image::GetVkImageLayout() const
    {
        return m_vkImageLayout;
    }

    uint32_t Image::CalculateImageMemorySize() const
    {
        uint32_t totalSize = 0;
        for (uint32_t mipLevel = 0; mipLevel < m_desc.m_mipCount; ++mipLevel)
        {
            const uint32_t mipSizeX = std::max<uint32_t>(1, m_desc.m_dimensions.x >> mipLevel);
            const uint32_t mipSizeY = std::max<uint32_t>(1, m_desc.m_dimensions.y >> mipLevel);
            const uint32_t mipSizeZ = std::max<uint32_t>(1, m_desc.m_dimensions.z >> mipLevel);
            const uint32_t mipBytes = ResourceFormatSize(m_desc.m_format, mipSizeX * mipSizeY * mipSizeZ);

            totalSize += mipBytes;
        }
        return totalSize;
    }

    bool Image::CreateVkImage()
    {
        if (m_desc.m_usageFlags == 0)
        {
            DX_LOG(Error, "Vulkan Image", "Image description with no usage flag set.");
            return false;
        }

        if (m_desc.m_nativeResource.has_value())
        {
            if (!m_desc.m_nativeResource->m_imageNativeResource)
            {
                DX_LOG(Error, "Vulkan Image", "Image description with invalid data.");
                return false;
            }
            else if (m_desc.m_nativeResource->m_ownsNativeResource && 
                !m_desc.m_nativeResource->m_imageMemoryNativeResource)
            {
                DX_LOG(Error, "Vulkan Image",
                    "Indicated that the image should own the resources but image memory was not provided.");
                return false;
            }

            m_vkImage = static_cast<VkImage>(m_desc.m_nativeResource->m_imageNativeResource);
            m_vkImageMemory = static_cast<VkDeviceMemory>(m_desc.m_nativeResource->m_imageMemoryNativeResource);

            // NOTE: Expected that the VkImage is already linked to the VkDeviceMemory.

            // When native resource are directly provided, the initial data will be ignored.
            if (m_desc.m_initialData != nullptr)
            {
                DX_LOG(Warning, "Vulkan Image", 
                    "Initial data provided will be ignored since the image native resources was directly provided.");
            }
            return true;
        }

        // If there is initial data to copy, use a staging image to transfer the data to the GPU image
        if (m_desc.m_initialData)
        {
            // Create staging source buffer to put data before transferring to GPU
            std::unique_ptr<Buffer> stageBuffer;
            {
                BufferDesc stageBufferDesc = {};
                stageBufferDesc.m_elementSizeInBytes = CalculateImageMemorySize();
                stageBufferDesc.m_elementCount = 1;
                stageBufferDesc.m_usageFlags = BufferUsage_TransferSrc;
                stageBufferDesc.m_memoryProperty = ResourceMemoryProperty::HostVisible;
                stageBufferDesc.m_initialData = m_desc.m_initialData;

                stageBuffer = std::make_unique<Buffer>(m_device, stageBufferDesc);
                if (!stageBuffer->Initialize())
                {
                    DX_LOG(Error, "Vulkan Image", "Failed to create Vulkan staging buffer.");
                    return false;
                }
            }

            // Create destination image in GPU
            {
                const VkImageUsageFlags vkImageUsageFlags =
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | // Destination of the transfer
                    ToVkImageUsageFlags(m_desc.m_usageFlags); // Actual final image usage

                // Memory properties we want:
                // - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: visible to the GPU only (optimal for GPU performance)
                const VkMemoryPropertyFlags vkMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

                if (!Utils::CreateVkImage(m_device,
                    ToVkImageType(m_desc.m_imageType),
                    m_desc.m_dimensions,
                    m_desc.m_mipCount,
                    ToVkFormat(m_desc.m_format),
                    ToVkImageTiling(m_desc.m_tiling),
                    vkImageUsageFlags,
                    vkMemoryProperties,
                    &m_vkImage,
                    &m_vkImageMemory))
                {
                    return false;
                }
            }

            // Transition image layout to be TRANSFER_DST_OPTIMAL for copy operation
            if (!Utils::TransitionImageLayout(m_device, this, 
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                // AFTER any point at the very start of the pipeline
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                // BEFORE it attempts to do a transfer write operation at the transfer stage of the pipeline
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT))
            {
                return false;
            }

            // Execute commands to copy staging buffer to destination image on GPU
            if (!Utils::CopyBufferToImage(m_device, this, stageBuffer.get()))
            {
                return false;
            }

            // Handle transition to final layout depending on its usage
            if (m_desc.m_usageFlags & ImageUsage_Sampled)
            {
                // Transition to be shader readable for shader usage
                if (!Utils::TransitionImageLayout(m_device, this,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    // AFTER it has finished to do writing operations in the transfer stage
                    // (basically when the copy buffer to image has finished)
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                    // BEFORE it attempts to do a shader read operation at the fragment shader stage of the pipeline
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT))
                {
                    return false;
                }
                m_vkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            else if (m_desc.m_usageFlags & ImageUsage_Storage)
            {
                // Transition to be general so it can be read/written in the shader
                if (!Utils::TransitionImageLayout(m_device, this,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                    // AFTER it has finished to do writing operations in the transfer stage
                    // (basically when the copy buffer to image has finished)
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                    // BEFORE it attempts to do a shader read/write operation at the fragment shader stage of the pipeline
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT))
                {
                    return false;
                }
                m_vkImageLayout = VK_IMAGE_LAYOUT_GENERAL;
            }
            else if (m_desc.m_usageFlags & ImageUsage_ColorAttachment)
            {
                DX_LOG(Error, "Vulkan Image", "Image usage is to as color attachment, but data was provided.");
            }
            else if (m_desc.m_usageFlags & ImageUsage_DepthStencilAttachment)
            {
                DX_LOG(Error, "Vulkan Image", "Image usage is to as depth stencil attachment, but data was provided.");
            }
        }
        // It there is no initial data to copy, just create image in GPU
        else
        {
            const VkImageUsageFlags vkImageUsageFlags = ToVkImageUsageFlags(m_desc.m_usageFlags);

            const VkMemoryPropertyFlags vkMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            if (!Utils::CreateVkImage(m_device,
                ToVkImageType(m_desc.m_imageType),
                m_desc.m_dimensions,
                m_desc.m_mipCount,
                ToVkFormat(m_desc.m_format),
                ToVkImageTiling(m_desc.m_tiling),
                vkImageUsageFlags,
                vkMemoryProperties,
                &m_vkImage,
                &m_vkImageMemory))
            {
                return false;
            }

            // Handle transition to final layout depending on its usage
            if (m_desc.m_usageFlags & ImageUsage_Sampled)
            {
                DX_LOG(Error, "Vulkan Image", "Image usage is to be sampled but not data was provided.");
            }
            else if (m_desc.m_usageFlags & ImageUsage_Storage)
            {
                // Transition to be general so it can be read/written in the shader
                if (!Utils::TransitionImageLayout(m_device, this,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                    // AFTER any point at the very start of the pipeline
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                    // BEFORE it attempts to do a shader read/write operation at the fragment shader stage of the pipeline
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT))
                {
                    return false;
                }
                m_vkImageLayout = VK_IMAGE_LAYOUT_GENERAL;
            }
            else if (m_desc.m_usageFlags & ImageUsage_ColorAttachment)
            {
                // Leave the layout as VK_IMAGE_LAYOUT_UNDEFINED and Render Pass will handle the transitions
                m_vkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            else if (m_desc.m_usageFlags & ImageUsage_DepthStencilAttachment)
            {
                // Leave the layout as VK_IMAGE_LAYOUT_UNDEFINED and Render Pass will handle the transitions
                m_vkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
        }

        DX_LOG(Verbose, "Vulkan Image", "Image %s %dx%dx%d and %d mipmaps created.",
            ImageTypeStr(m_desc.m_imageType), m_desc.m_dimensions.x, m_desc.m_dimensions.y, m_desc.m_dimensions.z, m_desc.m_mipCount);

        return true;
    }
} // namespace Vulkan
