#pragma once

#include <glm/mat4x4.hpp>

struct DrawContext;

class IRenderable
{
public:
	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) = 0;
};