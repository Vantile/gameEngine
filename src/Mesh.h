#pragma once

#include <IRenderable.h>

class Mesh : public IRenderable
{
public:
	void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;

private:
	glm::mat4 m_LocalTransform{ 1.f };
	glm::mat4 m_WorldTransform{ 1.f };
};