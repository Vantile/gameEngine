#pragma once

#include <entitysystem/component/TransformComponent.h>
#include <entitysystem/system/System.h>
#include <random>

class MovementSystem : public System
{
public:
	void Update(EntityRegistry& registry, FrameData& frameData) override
	{
		registry.GetView<TransformComponent>().Each([](TransformComponent& transform)
			{
				glm::vec3& position = transform.m_Position;
				std::mt19937 gen(std::random_device{}());
				constexpr float diff = 0.01f;
				std::uniform_real_distribution<float> xDist(position.x - diff, position.x + diff);
				std::uniform_real_distribution<float> yDist(position.y - diff, position.y + diff);
				position.x = xDist(gen);
				position.y = yDist(gen);
			});
	}
};