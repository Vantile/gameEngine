#pragma once

#include <IRenderable.h>

class Particle : public IRenderable
{
public:
	void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;

private:
	glm::mat4 m_Transform{ 1.f };
};