#pragma once

#include <entitysystem/component/Component.h>
#include <renderer/Vertex.h>

class RenderableComponent : public Component
{
public:
	enum RenderType
	{
		Point,
		Mesh	
	};

	RenderType m_RenderType = RenderType::Mesh;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
};