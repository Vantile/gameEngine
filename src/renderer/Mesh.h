#pragma once

#include <renderer/IRenderable.h>
#include <renderer/Vertex.h>
#include <renderer/VulkanTypes.h>
#include <vector>

class Mesh : public IRenderable
{
public:
	~Mesh();

	void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;

	void CreateVertexBuffer(VkDevice device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void CreateIndexBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void CreateStagingBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	void Randomize();

	std::vector<Vertex>& GetVertices() { return m_Vertices; }
	std::vector<uint32_t>& GetIndices() { return m_Indices; }

	const AllocatedBuffer& GetIndexBuffer() { return m_IndexBuffer; }
	const AllocatedBuffer& GetVertexBuffer() { return m_VertexBuffer; }
	const AllocatedBuffer& GetStagingBuffer() { return m_StagingBuffer; }
	const VkDeviceAddress& GetVertexBufferAddress() { return m_VertexBufferAddress; }

private:
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	AllocatedBuffer m_IndexBuffer;
	AllocatedBuffer m_VertexBuffer;
	VkDeviceAddress m_VertexBufferAddress;

	AllocatedBuffer m_StagingBuffer;
};