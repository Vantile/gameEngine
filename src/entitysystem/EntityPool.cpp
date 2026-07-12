#include <entitysystem/EntityPool.h>

EntityID EntityPool::Create()
{
	if (!m_FreeList.empty())
	{
		// Reuse free index
		uint32_t index = m_FreeList.back();
		m_FreeList.pop_back();
		return { index, m_Generations[index] };
	}

	m_Generations.push_back(0);
	return { (uint32_t)m_Generations.size() - 1, 0 };
}

void EntityPool::Destroy(EntityID id)
{
	++m_Generations[id.index];
	m_FreeList.push_back(id.index);
}

bool EntityPool::IsValid(EntityID id) const
{
	return id.index < m_Generations.size() && m_Generations[id.index] == id.generation;
}