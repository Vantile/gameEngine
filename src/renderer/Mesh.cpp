#include <renderer/Mesh.h>

#include <random>

void Mesh::Draw(const glm::mat4& topMatrix, DrawContext& ctx)
{

}

void Mesh::AllocateBuffers()
{

}

void Mesh::UpdateBuffers()
{

}

void Mesh::DestroyBuffers()
{

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