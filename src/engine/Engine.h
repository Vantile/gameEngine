#pragma once

#include <cassert>
#include <engine/FrameData.h>
#include <engine/Job.h>

class ECSManager;
class InputManager;
class VulkanRenderer;

struct EngineParams
{
	static constexpr uint32_t DEFAULT_WINDOW_WIDTH = 1200;
	static constexpr uint32_t DEFAULT_WINDOW_HEIGHT = 800;

	uint32_t windowWidth = DEFAULT_WINDOW_WIDTH;
	uint32_t windowHeight = DEFAULT_WINDOW_HEIGHT;
};

class Engine
{
public:
	Engine();
	~Engine();
	void Init();
	void Run();
	void Cleanup();

	static Engine& GetInstance() { assert(m_Instance != nullptr); return *m_Instance; }

	ECSManager& GetECSManager() { assert(m_ECSManager != nullptr); return *m_ECSManager; }

	JobSystem& GetJobSystem() { assert(m_JobSystem != nullptr); return *m_JobSystem; }

	EngineParams& GetEngineParams() { return m_EngineParams; }

private:
	inline static Engine* m_Instance;
	FrameData m_FrameData;
	EngineParams m_EngineParams{};

	ECSManager* m_ECSManager = nullptr;
	InputManager* m_InputManager = nullptr;
	VulkanRenderer* m_Renderer = nullptr;
	JobSystem* m_JobSystem = nullptr;
};