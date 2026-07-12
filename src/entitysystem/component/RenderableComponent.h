#pragma once

#include <entitysystem/component/Component.h>

class RenderableComponent : public Component
{
public:
	enum RenderType
	{
		Point,
		Mesh	
	};

	RenderType m_RenderType = RenderType::Mesh;
};