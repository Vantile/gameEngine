#pragma once

#include <entitysystem/Entity.h>
#include <glm/vec3.hpp>
#include <renderer/Vertex.h>
#include <vector>

struct RenderObject
{
	bool point{ false };
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	EntityID entityID;
};

struct FrameData
{
	std::vector<RenderObject> renderObjects;

	void Reset()
	{
		renderObjects.clear();
	}
};