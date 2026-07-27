#include <engine/Engine.h>

#include <entitysystem/ECSManager.h>
#include <input/InputManager.h>
#include <memory/Memory.h>
#include <renderer/VulkanRenderer.h>
#include <SDL3/SDL.h>
#include <tracy/Tracy.hpp>

Engine::Engine() = default;
Engine::~Engine() = default;

void Engine::Init()
{
    tracy::SetThreadName("Main Thread");

    m_Instance = this;

    m_ECSManager = engineNew(ECSManager);
    m_InputManager = engineNew(InputManager);
    m_Renderer = engineNew(VulkanRenderer);
    m_JobSystem = engineNew(JobSystem);

    constexpr size_t threadCount = 4;
    m_ECSManager->Init();
    m_InputManager->Init();
    m_Renderer->Init(threadCount);
    m_JobSystem->Init(threadCount);
}

void Engine::Run()
{
    SDL_Event e;
    bool bQuit = false;

    // Main loop
    while (!bQuit)
    {
        ZoneScoped;
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            // Close the window when user alt-f4s or clicks the X button
            if (e.type == SDL_EVENT_QUIT)
            {
                bQuit = true;
            }

            m_InputManager->ProcessSDLEvent(e);
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

        FrameMark;
    }
}

void Engine::Cleanup()
{
    m_ECSManager->Cleanup();
    m_InputManager->Cleanup();
    m_Renderer->Cleanup();
    m_JobSystem->Shutdown();

    engineDelete(m_ECSManager);
    engineDelete(m_InputManager);
    engineDelete(m_Renderer);
    engineDelete(m_JobSystem);

    m_Instance = nullptr;
}