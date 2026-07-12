#pragma once

#include <entitysystem/Entity.h>
#include <vector>

class EntityPool
{
public:
	EntityID Create();
	void Destroy(EntityID id);
	bool IsValid(EntityID id) const;

private:
	std::vector<uint32_t> m_FreeList;
	std::vector<uint8_t> m_Generations;
};