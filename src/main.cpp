#include <entitysystem/ECSManager.h>
#include <renderer/VulkanRenderer.h>
#include <SDL3/SDL.h>

int main(int argc, char* argv[])
{
	ECSManager manager;
	VulkanRenderer renderer;

	manager.Init();
	renderer.Init();
	
    SDL_Event e;
    bool bQuit = false;

    // Main loop
    while (!bQuit)
    {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            // Close the window when user alt-f4s or clicks the X button
            if (e.type == SDL_EVENT_QUIT)
            {
                bQuit = true;
            }

            renderer.ProcessSDLEvent(e);
        }

        manager.Run();
        renderer.Run();
    }

    manager.Cleanup();
	renderer.Cleanup();

	return 0;
}
