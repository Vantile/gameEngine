#include <entitysystem/ECSManager.h>

#include <entitysystem/EntityRegistry.h>
#include <entitysystem/system/MovementSystem.h>
#include <entitysystem/system/RenderSystem.h>

void ECSManager::Init()
{
	m_Registry = std::make_unique<EntityRegistry>();

	m_Systems.push_back(std::make_unique<MovementSystem>());
	m_Systems.push_back(std::make_unique<RenderSystem>());

	// Create entities
	EntityID entity1 = m_Registry->CreateEntity();
	EntityID entity2 = m_Registry->CreateEntity();
	EntityID entity3 = m_Registry->CreateEntity();

	TransformComponent transform;
	RenderableComponent renderable;
	renderable.m_RenderType = RenderableComponent::RenderType::Mesh;
	m_Registry->Emplace(entity1, transform);
	m_Registry->Emplace(entity1, renderable);

	renderable.m_RenderType = RenderableComponent::RenderType::Point;
	m_Registry->Emplace(entity2, transform);
	m_Registry->Emplace(entity2, renderable);

	m_Registry->Emplace(entity3, transform);
	m_Registry->Emplace(entity3, renderable);
}

void ECSManager::Run()
{
	for (std::unique_ptr<System>& system : m_Systems)
	{
		system->Update(*m_Registry.get());
	}
}

void ECSManager::Cleanup()
{
	m_Systems.clear();
	m_Registry.release();
}