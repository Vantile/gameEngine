#pragma once

#include <entitysystem/component/Component.h>
#include <glm/vec3.hpp>

class TransformComponent : public Component
{
public:
	glm::vec3 m_Position{ 0.f, 0.f, 0.f };
};