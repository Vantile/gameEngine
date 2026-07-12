#include <engine/Engine.h>

#include <SDL3/SDL.h>

void Engine::Init()
{
	ECSManager manager;
	VulkanRenderer renderer;

    m_ECSManager = std::make_unique<ECSManager>();
    m_Renderer = std::make_unique<VulkanRenderer>();

    m_ECSManager->Init();
    m_Renderer->Init();
}

void Engine::Run()
{
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

            m_Renderer->ProcessSDLEvent(e);
        }

        m_FrameData.Reset();
        m_ECSManager->Run(m_FrameData);
        m_Renderer->Run(m_FrameData);
    }
}

void Engine::Cleanup()
{
    m_ECSManager->Cleanup();
    m_Renderer->Cleanup();

    m_ECSManager.release();
    m_Renderer.release();
}