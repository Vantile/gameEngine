#pragma once

#include <engine/Engine.h>
#include <entitysystem/component/TransformComponent.h>
#include <entitysystem/system/System.h>
#include <random>

class MovementSystem : public System
{
public:
	void Update(EntityRegistry& registry, FrameData& frameData) override
	{
		ZoneScoped;
		JobCounter systemCounter;
		JobSystem& jobSystem = Engine::GetInstance().GetJobSystem();
		registry.GetView<TransformComponent>().Each([&systemCounter, &jobSystem](TransformComponent& transform)
			{
				jobSystem.Submit({
					[&transform]() {
						ZoneScopedN("MovementSystem::Update Job");
						glm::vec3& position = transform.m_Position;
						std::mt19937 gen(std::random_device{}());
						constexpr float diff = 0.001f;
						std::uniform_real_distribution<float> xDist(position.x - diff, position.x + diff);
						std::uniform_real_distribution<float> yDist(position.y - diff, position.y + diff);
						position.x = xDist(gen);
						position.y = yDist(gen);
					}
				}, &systemCounter);
			});

		{
			ZoneScopedN("MovementSystem::Update Wait");
			systemCounter.Wait();
		}
	}
};