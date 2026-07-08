#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Vertex
{
	glm::vec3 m_Position;
	float m_UvX;
	glm::vec3 m_Normal;
	float m_UvY;
	glm::vec4 m_Color;
};