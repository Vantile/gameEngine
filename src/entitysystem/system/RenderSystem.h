#pragma once

#include <entitysystem/component/RenderableComponent.h>
#include <entitysystem/component/TransformComponent.h>
#include <entitysystem/system/System.h>

class RenderSystem : public System
{
public:
	void Update(EntityRegistry& registry) override
	{
		registry.GetView<TransformComponent, RenderableComponent>().Each([](TransformComponent& transform, RenderableComponent& renderable)
			{

			});
	}
};