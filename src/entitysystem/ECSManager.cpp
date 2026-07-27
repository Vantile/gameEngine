#include <entitysystem/ECSManager.h>

#include <entitysystem/EntityRegistry.h>
#include <entitysystem/system/MovementSystem.h>
#include <entitysystem/system/RenderSystem.h>
#include <memory/Memory.h>
#include <tracy/Tracy.hpp>

void ECSManager::Init()
{
	m_Registry = engineNew(EntityRegistry);

	m_Systems.push_back(engineNew(MovementSystem));
	m_Systems.push_back(engineNew(RenderSystem));

	// Create entities
	EntityID entity1 = m_Registry->CreateEntity();

	TransformComponent transform;
	RenderableComponent renderableMesh;
	renderableMesh.m_RenderType = RenderableComponent::RenderType::Mesh;

	std::vector<Vertex> vertices;
	vertices.resize(4);

	vertices[0].m_Position = { 0.7, -0.5, 0 };
	vertices[1].m_Position = { 0.5, 0.3, 0 };
	vertices[2].m_Position = { -0.5, -0.5, 0 };
	vertices[3].m_Position = { -0.5, 0.5, 0 };

	vertices[0].m_Color = { 0, 0, 0, 1 };
	vertices[1].m_Color = { 0.5, 0.5, 0.5, 1 };
	vertices[2].m_Color = { 1, 0, 0, 1 };
	vertices[3].m_Color = { 0, 1, 0, 1 };

	std::vector<uint32_t> indices;
	indices.resize(6);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;

	renderableMesh.m_Vertices = std::move(vertices);
	renderableMesh.m_Indices = std::move(indices);

	m_Registry->Emplace(entity1, transform);
	m_Registry->Emplace(entity1, renderableMesh);

	std::mt19937 gen(std::random_device{}());
	std::uniform_real_distribution<float> dist(-1.f, 1.f);
	
	constexpr uint32_t numPoints = 1000;
	RenderableComponent renderablePoint;
	renderablePoint.m_RenderType = RenderableComponent::RenderType::Point;
	for (int i = 0; i < numPoints; ++i)
	{
		transform.m_Position.x = dist(gen);
		transform.m_Position.y = dist(gen);
		EntityID newEntity = m_Registry->CreateEntity();
		m_Registry->Emplace(newEntity, transform);
		m_Registry->Emplace(newEntity, renderablePoint);
	}
}

void ECSManager::Run(FrameData& frameData)
{
	ZoneScoped;

	RenderableComponent renderablePoint;
	renderablePoint.m_RenderType = RenderableComponent::RenderType::Point;
	while (!m_SpawnQueue.empty())
	{
		glm::vec3& spawnPosition = m_SpawnQueue.front();
		m_SpawnQueue.pop();
		TransformComponent transform;
		transform.m_Position = std::move(spawnPosition);
		EntityID newEntity = m_Registry->CreateEntity();
		m_Registry->Emplace(newEntity, std::move(transform));
		m_Registry->Emplace(newEntity, renderablePoint);
	}

	for (System* system : m_Systems)
	{
		assert(system != nullptr);
		system->Update(*m_Registry, frameData);
	}
}

void ECSManager::Cleanup()
{
	for (System* system : m_Systems)
	{
		engineDelete(system);
	}
	m_Systems.clear();

	engineDelete(m_Registry);
}