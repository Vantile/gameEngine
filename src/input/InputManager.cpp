#include <input/InputManager.h>

#include <engine/Engine.h>
#include <entitysystem/ECSManager.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

glm::vec3 ScreenToWorld(glm::vec2 screenPos, float depth, /*const glm::mat4& view, const glm::mat4& proj,*/ glm::vec2 screenSize)
{
	// Convert screen position to NDC [-1, 1]
	glm::vec4 ndc {
		(screenPos.x / screenSize.x) * 2.f - 1.f,
		(screenPos.y / screenSize.y) * 2.f - 1.f,
		depth,
		1.f
	};

	// Not needed since we have 2D flat, no perspective
	//{
	//	// Unproject through inverse view-projection
	//	glm::mat4 invViewProj = glm::inverse(proj * view);
	//	glm::vec4 worldPos = invViewProj * ndc;
	//	worldPos /= worldPos.w;  // perspective divide
	//}
	
	return glm::vec3(std::move(ndc));
}

void InputManager::Init()
{

}

void InputManager::ProcessSDLEvent(SDL_Event& event)
{
	if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
	{
		glm::vec2 screenPos = { event.button.x, event.button.y };

		EngineParams& engineParams = Engine::GetInstance().GetEngineParams();
		glm::vec2 screenSize = { engineParams.windowWidth, engineParams.windowHeight };

		glm::vec3 worldPos = ScreenToWorld(screenPos, 0.f, screenSize);

		Engine::GetInstance().GetECSManager().QueueEntitySpawn(std::move(worldPos));
	}
}

void InputManager::Cleanup()
{

}