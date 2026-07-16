#include <renderer/RenderPoint.h>

#include <format>
#include <renderer/VulkanMemoryManager.h>

RenderPoint::~RenderPoint()
{
    VulkanMemoryManager::GetInstance().DestroyBuffer(m_VertexBuffer.m_Buffer, m_VertexBuffer.m_Allocation);

    if (m_StagingBuffer.m_Buffer != nullptr)
    {
        VulkanMemoryManager::GetInstance().DestroyBuffer(m_StagingBuffer.m_Buffer, m_StagingBuffer.m_Allocation);
    }
}

void RenderPoint::Draw(const glm::mat4& topMatrix, DrawContext& ctx)
{

}

void RenderPoint::CreateVertexBuffer(VkDevice device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = memoryUsage;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VulkanMemoryManager::GetInstance().AllocateBuffer(&bufferInfo, &vmaAllocInfo, &m_VertexBuffer.m_Buffer, &m_VertexBuffer.m_Allocation, &m_VertexBuffer.m_AllocationInfo);

    static uint32_t id = 0;
    VulkanMemoryManager::GetInstance().SetAllocationName(m_VertexBuffer.m_Allocation, "RenderPoint VertexBuffer {}");

    VkBufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = m_VertexBuffer.m_Buffer;
    m_VertexBufferAddress = vkGetBufferDeviceAddress(device, &deviceAddressInfo);
}

void RenderPoint::CreateStagingBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = memoryUsage;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VulkanMemoryManager::GetInstance().AllocateBuffer(&bufferInfo, &vmaAllocInfo, &m_StagingBuffer.m_Buffer, &m_StagingBuffer.m_Allocation, &m_StagingBuffer.m_AllocationInfo);

    VulkanMemoryManager::GetInstance().SetAllocationName(m_VertexBuffer.m_Allocation, "RenderPoint StagingBuffer");
}