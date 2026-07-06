#include <VulkanUtils.h>

#include <fstream>
#include <vector>

namespace VulkanUtils
{
	namespace Image
	{
		VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
		{
            VkImageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.pNext = nullptr;

            info.imageType = VK_IMAGE_TYPE_2D;

            info.format = format;
            info.extent = extent;

            info.mipLevels = 1;
            info.arrayLayers = 1;

            // For MSAA; for now, it's not enabled
            info.samples = VK_SAMPLE_COUNT_1_BIT;

            // Optimal tiling, which means the image is stored on the best GPU format
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.usage = usageFlags;

            return info;
		}

        VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
        {
            VkImageViewCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.pNext = nullptr;

            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.image = image;
            info.format = format;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;
            info.subresourceRange.aspectMask = aspectFlags;

            return info;
        }

        VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask)
        {
            VkImageSubresourceRange subImage{};
            subImage.aspectMask = aspectMask;
            subImage.baseMipLevel = 0;
            subImage.levelCount = VK_REMAINING_MIP_LEVELS;
            subImage.baseArrayLayer = 0;
            subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

            return subImage;
        }

        void TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
        {
            VkImageMemoryBarrier2 imageBarrier{};
            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            imageBarrier.pNext = nullptr;

            // Setting the stage mask to ALL_COMMANDS is inefficient; if you are doing many transitions per frame as part
            // of a post-process chain, you want to avoid doing this and use StageMasks more accurate
            imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
            imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

            imageBarrier.oldLayout = currentLayout;
            imageBarrier.newLayout = newLayout;

            VkImageAspectFlags aspectMask = newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                ? VK_IMAGE_ASPECT_DEPTH_BIT
                : VK_IMAGE_ASPECT_COLOR_BIT;
            imageBarrier.subresourceRange = ImageSubresourceRange(aspectMask);
            imageBarrier.image = image;

            VkDependencyInfo depInfo{};
            depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            depInfo.pNext = nullptr;

            depInfo.imageMemoryBarrierCount = 1;
            depInfo.pImageMemoryBarriers = &imageBarrier;

            vkCmdPipelineBarrier2(commandBuffer, &depInfo);
        }

        void CopyImageToImage(VkCommandBuffer commandBuffer, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize)
        {
            VkImageBlit2 blitRegion{};
            blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
            blitRegion.pNext = nullptr;

            blitRegion.srcOffsets[1].x = srcSize.width;
            blitRegion.srcOffsets[1].y = srcSize.height;
            blitRegion.srcOffsets[1].z = 1;

            blitRegion.dstOffsets[1].x = dstSize.width;
            blitRegion.dstOffsets[1].y = dstSize.height;
            blitRegion.dstOffsets[1].z = 1;

            blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitRegion.srcSubresource.baseArrayLayer = 0;
            blitRegion.srcSubresource.layerCount = 1;
            blitRegion.srcSubresource.mipLevel = 0;

            blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitRegion.dstSubresource.baseArrayLayer = 0;
            blitRegion.dstSubresource.layerCount = 1;
            blitRegion.dstSubresource.mipLevel = 0;

            VkBlitImageInfo2 blitInfo{};
            blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
            blitInfo.pNext = nullptr;
            blitInfo.dstImage = destination;
            blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            blitInfo.srcImage = source;
            blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            blitInfo.filter = VK_FILTER_LINEAR;
            blitInfo.regionCount = 1;
            blitInfo.pRegions = &blitRegion;

            vkCmdBlitImage2(commandBuffer, &blitInfo);
        }
	}

    namespace Command
    {
        VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
        {
            VkCommandPoolCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.pNext = nullptr;
            info.queueFamilyIndex = queueFamilyIndex;
            info.flags = flags;
            return info;
        }

        VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count)
        {
            VkCommandBufferAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.pNext = nullptr;

            info.commandPool = pool;
            info.commandBufferCount = count;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            return info;
        }

        VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
        {
            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.pNext = nullptr;
            info.pInheritanceInfo = nullptr;
            info.flags = flags;
            return info;
        }

        VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd)
        {
            VkCommandBufferSubmitInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            info.pNext = nullptr;
            info.commandBuffer = cmd;
            info.deviceMask = 0;
            return info;
        }
    }

    namespace Sync
    {
        VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags)
        {
            VkFenceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = flags;
            return info;
        }

        VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags)
        {
            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = flags;
            return info;
        }

        VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
        {
            VkSemaphoreSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            submitInfo.pNext = nullptr;
            submitInfo.semaphore = semaphore;
            submitInfo.stageMask = stageMask;
            submitInfo.deviceIndex = 0;
            submitInfo.value = 1;
            return submitInfo;
        }

        VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
        {
            VkSubmitInfo2 info{};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
            info.pNext = nullptr;
            info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
            info.pWaitSemaphoreInfos = waitSemaphoreInfo;
            info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
            info.pSignalSemaphoreInfos = signalSemaphoreInfo;
            info.commandBufferInfoCount = 1;
            info.pCommandBufferInfos = cmd;
            return info;
        }
    }

    namespace Shader
    {
        bool LoadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule)
        {
            // Open the file with cursor at the end
            std::ifstream file(filePath, std::ios::ate | std::ios::binary);
            if (!file.is_open())
            {
                return false;
            }

            // Since the cursor is at the end of the file, check its location to know the file size
            size_t fileSize = (size_t)file.tellg();

            // SPIR-V expects the buffer to be on uint32, reserve enough space
            std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

            // Reset file cursor at the beginning of the file
            file.seekg(0);
            file.read((char*)buffer.data(), fileSize);
            file.close();

            // Create a new shader module using the buffer we loaded
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.pNext = nullptr;
            // Codesize has to be in bytes, multiply the ints in the buffer by the size of an int
            createInfo.codeSize = buffer.size() * sizeof(uint32_t);
            createInfo.pCode = buffer.data();

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            {
                return false;
            }

            *outShaderModule = shaderModule;
            return true;
        }
    }
}