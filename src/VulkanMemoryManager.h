#pragma once

#include <vk_mem_alloc.h>

class VulkanMemoryManager
{
public:
	void InitAllocator(const VkPhysicalDevice& chosenGPU, const VkDevice& device, const VkInstance& instance);
	void DestroyAllocator();

	void AllocateImage(const VkImageCreateInfo* imageInfo, const VmaAllocationCreateInfo* allocationInfo, VkImage* image, VmaAllocation* allocation);
	void DestroyImage(VkImage image, VmaAllocation allocation);

	void AllocateBuffer(const VkBufferCreateInfo* bufferInfo, const VmaAllocationCreateInfo* allocationCreateInfo, VkBuffer* buffer, VmaAllocation* allocation, VmaAllocationInfo* allocationInfo);
	void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

private:
	VmaAllocator m_Allocator;
};