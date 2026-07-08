#include <Mesh.h>

void Mesh::Draw(const glm::mat4& topMatrix, DrawContext& ctx)
{
	glm::mat4 matrix = topMatrix * m_WorldTransform;
}