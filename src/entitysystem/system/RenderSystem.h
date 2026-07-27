#pragma once

#include <entitysystem/component/RenderableComponent.h>
#include <entitysystem/component/TransformComponent.h>
#include <entitysystem/system/System.h>

class RenderSystem : public System
{
public:
	void Update(EntityRegistry& registry, FrameData& frameData) override
	{
		ZoneScoped;
		registry.GetView<TransformComponent, RenderableComponent>().Each([&frameData](TransformComponent& transform, RenderableComponent& renderable)
		{
			RenderObject object;
			object.point = renderable.m_RenderType == RenderableComponent::RenderType::Point;
			object.entityID = renderable.GetOwner();

			if (object.point)
			{
				Vertex vertex = {};
				vertex.m_Position = transform.m_Position;
				vertex.m_Color = { 1.f, 1.f, 0.f, 1.f };
				object.vertices.push_back(vertex);
			}
			else
			{
				object.vertices = renderable.m_Vertices;
				object.indices = renderable.m_Indices;
			}

			frameData.renderObjects.push_back(std::move(object));
		});
	}
};