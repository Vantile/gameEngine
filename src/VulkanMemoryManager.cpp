#include <VulkanMemoryManager.h>

#include <VulkanUtils.h>

void VulkanMemoryManager::InitAllocator(const VkPhysicalDevice& chosenGPU, const VkDevice& device, const VkInstance& instance)
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = chosenGPU;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &m_Allocator);
}

void VulkanMemoryManager::DestroyAllocator()
{
    vmaDestroyAllocator(m_Allocator);
}

void VulkanMemoryManager::AllocateImage(const VkImageCreateInfo* imageInfo, const VmaAllocationCreateInfo* allocationCreateInfo, VkImage* image, VmaAllocation* allocation)
{
    VULKAN_CHECK(vmaCreateImage(m_Allocator, imageInfo, allocationCreateInfo, image, allocation, nullptr));
}

void VulkanMemoryManager::DestroyImage(VkImage image, VmaAllocation allocation)
{
    vmaDestroyImage(m_Allocator, image, allocation);
}

void VulkanMemoryManager::AllocateBuffer(const VkBufferCreateInfo* bufferInfo, const VmaAllocationCreateInfo* allocationCreateInfo, VkBuffer* buffer, VmaAllocation* allocation, VmaAllocationInfo* allocationInfo)
{
    VULKAN_CHECK(vmaCreateBuffer(m_Allocator, bufferInfo, allocationCreateInfo, buffer, allocation, allocationInfo));
}

void VulkanMemoryManager::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
{
    vmaDestroyBuffer(m_Allocator, buffer, allocation);
}