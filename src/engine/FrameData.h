#pragma once

#include <glm/vec3.hpp>

struct RenderObject
{
	bool point{ false };
	glm::vec3 position{ 0.f, 0.f, 0.f };
};

struct FrameData
{
	std::vector<RenderObject> renderObjects;

	void Reset()
	{
		renderObjects.clear();
	}
};