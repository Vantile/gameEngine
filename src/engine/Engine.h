#pragma once

#include <cassert>
#include <engine/FrameData.h>
#include <engine/Job.h>
#include <memory>

class ECSManager;
class VulkanRenderer;

class Engine
{
public:
	Engine();
	~Engine();
	void Init();
	void Run();
	void Cleanup();

	static Engine& GetInstance() { assert(m_Instance != nullptr); return *m_Instance; }

	JobSystem& GetJobSystem() { assert(m_JobSystem != nullptr); return *m_JobSystem.get(); }

private:
	inline static Engine* m_Instance;
	FrameData m_FrameData;
	std::unique_ptr<ECSManager> m_ECSManager;
	std::unique_ptr<VulkanRenderer> m_Renderer;
	std::unique_ptr<JobSystem> m_JobSystem;
};