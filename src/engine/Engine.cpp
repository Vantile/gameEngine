#include <engine/Engine.h>

#include <entitysystem/ECSManager.h>
#include <renderer/VulkanRenderer.h>
#include <SDL3/SDL.h>
#define TRACY_ENABLE
#include <tracy/Tracy.hpp>

Engine::Engine() = default;
Engine::~Engine() = default;

void Engine::Init()
{
    m_ECSManager = std::make_unique<ECSManager>();
    m_Renderer = std::make_unique<VulkanRenderer>();
    m_JobSystem = std::make_unique<JobSystem>();

    m_ECSManager->Init();
    m_Renderer->Init();
    m_JobSystem->Init();

    m_Instance = this;
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

        JobCounter simCounter;
        m_FrameData.Reset();

        m_JobSystem->Submit({
            [this]() {
                m_ECSManager->Run(m_FrameData);
            } 
        }, &simCounter);

        simCounter.Wait();

        m_JobSystem->Submit({
            [this]() {
                m_Renderer->Run(m_FrameData);
            }
        }, &simCounter);

        simCounter.Wait();

        //m_ECSManager->Run(m_FrameData);
        //m_Renderer->Run(m_FrameData);

        FrameMark;
    }
}

void Engine::Cleanup()
{
    m_ECSManager->Cleanup();
    m_Renderer->Cleanup();
    m_JobSystem->Shutdown();

    m_ECSManager.release();
    m_Renderer.release();
    m_JobSystem.release();

    m_Instance = nullptr;
}