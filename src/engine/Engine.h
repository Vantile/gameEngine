#pragma once

#include <entitysystem/ECSManager.h>
#include <memory>
#include <renderer/VulkanRenderer.h>

class ECSManager;
class VulkanRenderer;

class Engine
{
public:
	void Init();
	void Run();
	void Cleanup();

private:
	FrameData m_FrameData;
	std::unique_ptr<ECSManager> m_ECSManager;
	std::unique_ptr<VulkanRenderer> m_Renderer;
};