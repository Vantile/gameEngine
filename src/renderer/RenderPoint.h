#pragma once

#include <renderer/IRenderable.h>
#include <renderer/Vertex.h>
#include <renderer/VulkanTypes.h>

class RenderPoint : public IRenderable
{
public:
	~RenderPoint();

	void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;

	void CreateVertexBuffer(VkDevice device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void CreateStagingBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	Vertex& GetVertex() { return m_Vertex; }
	const Vertex& GetVertex() const { return m_Vertex; }

	const AllocatedBuffer& GetVertexBuffer() { return m_VertexBuffer; }
	const VkDeviceAddress& GetVertexBufferAddress() { return m_VertexBufferAddress; }
	const AllocatedBuffer& GetStagingBuffer() { return m_StagingBuffer; }

private:
	Vertex m_Vertex;

	AllocatedBuffer m_VertexBuffer;
	VkDeviceAddress m_VertexBufferAddress;

	AllocatedBuffer m_StagingBuffer;
};