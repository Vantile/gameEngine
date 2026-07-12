#pragma once

#include <renderer/IRenderable.h>
#include <renderer/Vertex.h>
#include <renderer/VulkanTypes.h>

class RenderPoint : public IRenderable
{
public:
	void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;

	Vertex& GetVertex() { return m_Vertex; }

	AllocatedBuffer& GetVertexBuffer() { return m_VertexBuffer; }
	VkDeviceAddress& GetVertexBufferAddress() { return m_VertexBufferAddress; }

private:
	Vertex m_Vertex;

	AllocatedBuffer m_VertexBuffer;
	VkDeviceAddress m_VertexBufferAddress;
};