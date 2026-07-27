#pragma once

#include <SDL3/SDL.h>

class InputManager
{
public:
	void Init();
	void ProcessSDLEvent(SDL_Event& event);
	void Cleanup();
};