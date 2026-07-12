#pragma once

#include <entitysystem/EntityRegistry.h>
#include <entitysystem/system/System.h>
#include <memory>
#include <vector>

class System;

class ECSManager
{
public:
	void Init();
	void Run(FrameData& frameData);
	void Cleanup();

private:
	std::unique_ptr<EntityRegistry> m_Registry;
	std::vector<std::unique_ptr<System>> m_Systems;
};