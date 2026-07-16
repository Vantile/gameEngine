#pragma once

#include <cassert>
#include <vk_mem_alloc.h>

class VulkanMemoryManager
{
public:
	static VulkanMemoryManager& GetInstance() { assert(m_Instance != nullptr); return *m_Instance; }

	void InitAllocator(const VkPhysicalDevice& chosenGPU, const VkDevice& device, const VkInstance& instance);
	void DestroyAllocator();

	void AllocateImage(const VkImageCreateInfo* imageInfo, const VmaAllocationCreateInfo* allocationInfo, VkImage* image, VmaAllocation* allocation);
	void DestroyImage(VkImage image, VmaAllocation allocation);

	void AllocateBuffer(const VkBufferCreateInfo* bufferInfo, const VmaAllocationCreateInfo* allocationCreateInfo, VkBuffer* buffer, VmaAllocation* allocation, VmaAllocationInfo* allocationInfo);
	void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

	void SetAllocationName(VmaAllocation allocation, const char* name) { vmaSetAllocationName(m_Allocator, allocation, name); }

private:
	inline static VulkanMemoryManager* m_Instance;
	VmaAllocator m_Allocator;
};