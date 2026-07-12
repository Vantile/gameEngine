#pragma once

#include <cassert>
#include <vulkan/vulkan.h>

#define VULKAN_CHECK(x)			\
	{							\
		VkResult err = x;       \
		if (err != VK_SUCCESS)  \
		{                       \
			assert(false);      \
		}                       \
	}

namespace VulkanUtils
{
	namespace Image
	{
		VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
		VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

		void TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
		void CopyImageToImage(VkCommandBuffer commandBuffer, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
	}

	namespace Command
	{
		VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1);
		VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
		VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);
	}

	namespace Sync
	{
		VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);
		VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
		VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
		VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
	}

	namespace Shader
	{
		bool LoadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule);
	}

	namespace Render
	{
		VkRenderingAttachmentInfo RenderingAttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfo RenderingDepthAttachmentInfo(VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingInfo RenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment);
	}

	namespace Pipeline
	{
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();
		VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entry = "main");
	}
}