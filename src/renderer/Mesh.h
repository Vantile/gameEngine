#pragma once

#include <renderer/IRenderable.h>
#include <renderer/Vertex.h>
#include <renderer/VulkanTypes.h>
#include <vector>

class Mesh : public IRenderable
{
public:
	void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;

	void AllocateBuffers();
	void UpdateBuffers();
	void DestroyBuffers();

	void Randomize();

	std::vector<Vertex>& GetVertices() { return m_Vertices; }
	std::vector<uint32_t>& GetIndices() { return m_Indices; }

	AllocatedBuffer& GetIndexBuffer() { return m_IndexBuffer; }
	AllocatedBuffer& GetVertexBuffer() { return m_VertexBuffer; }
	VkDeviceAddress& GetVertexBufferAddress() { return m_VertexBufferAddress; }

private:
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	AllocatedBuffer m_IndexBuffer;
	AllocatedBuffer m_VertexBuffer;
	VkDeviceAddress m_VertexBufferAddress;
};