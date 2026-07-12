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
				std::uniform_real_distribution<float> xDist(position.x - 0.2f, position.x + 0.2f);
				std::uniform_real_distribution<float> yDist(position.y - 0.2f, position.y + 0.2f);
				position.x = xDist(gen);
				position.y = yDist(gen);
			});
	}
};