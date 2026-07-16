#include <renderer/Mesh.h>

#include <random>
#include <renderer/VulkanMemoryManager.h>

Mesh::~Mesh()
{
    VulkanMemoryManager::GetInstance().DestroyBuffer(m_VertexBuffer.m_Buffer, m_VertexBuffer.m_Allocation);
    VulkanMemoryManager::GetInstance().DestroyBuffer(m_IndexBuffer.m_Buffer, m_IndexBuffer.m_Allocation);

    if (m_StagingBuffer.m_Buffer != nullptr)
    {
        VulkanMemoryManager::GetInstance().DestroyBuffer(m_StagingBuffer.m_Buffer, m_StagingBuffer.m_Allocation);
    }
}

void Mesh::Draw(const glm::mat4& topMatrix, DrawContext& ctx)
{

}

void Mesh::CreateVertexBuffer(VkDevice device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
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

    VkBufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = m_VertexBuffer.m_Buffer;
    m_VertexBufferAddress = vkGetBufferDeviceAddress(device, &deviceAddressInfo);
}

void Mesh::CreateIndexBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = memoryUsage;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VulkanMemoryManager::GetInstance().AllocateBuffer(&bufferInfo, &vmaAllocInfo, &m_IndexBuffer.m_Buffer, &m_IndexBuffer.m_Allocation, &m_IndexBuffer.m_AllocationInfo);
}

void Mesh::CreateStagingBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
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
}

void Mesh::Randomize()
{
	std::mt19937 gen(std::random_device{}());
	std::uniform_real_distribution<float> colorDist(0.f, 1.f);
	std::uniform_real_distribution<float> posDist(-1.f, 1.f);
	std::uniform_real_distribution<float> depthDist(0.f, 1.f);
	for (Vertex& vertex : m_Vertices)
	{
		vertex.m_Color = { colorDist(gen), colorDist(gen), colorDist(gen), 1 };
		vertex.m_Position = { posDist(gen), posDist(gen), depthDist(gen) };
	}
}