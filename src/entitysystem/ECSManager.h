#pragma once

#include <entitysystem/EntityRegistry.h>
#include <entitysystem/system/System.h>
#include <queue>
#include <vector>

class System;

class ECSManager
{
public:
	void Init();
	void Run(FrameData& frameData);
	void Cleanup();

	void QueueEntitySpawn(glm::vec3 entityPos) { m_SpawnQueue.push(std::move(entityPos)); }

private:
	std::queue<glm::vec3> m_SpawnQueue;

	EntityRegistry* m_Registry;
	std::vector<System*> m_Systems;
};