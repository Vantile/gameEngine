#pragma once

#include <entitysystem/component/RenderableComponent.h>
#include <entitysystem/component/TransformComponent.h>
#include <entitysystem/system/System.h>

class RenderSystem : public System
{
public:
	void Update(EntityRegistry& registry, FrameData& frameData) override
	{
		registry.GetView<TransformComponent, RenderableComponent>().Each([&frameData](TransformComponent& transform, RenderableComponent& renderable)
			{
				RenderObject object;
				object.point = renderable.m_RenderType == RenderableComponent::RenderType::Point;
				object.position = transform.m_Position;
				frameData.renderObjects.push_back(std::move(object));
			});
	}
};