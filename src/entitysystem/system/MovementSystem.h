#pragma once

#include <entitysystem/component/TransformComponent.h>
#include <entitysystem/system/System.h>

class MovementSystem : public System
{
public:
	void Update(EntityRegistry& registry) override
	{
		registry.GetView<TransformComponent>().Each([](TransformComponent& transform)
			{

			});
	}
};