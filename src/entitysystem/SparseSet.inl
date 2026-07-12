#include <cassert>

template<typename T>
void SparseSet<T>::Insert(EntityID id, T component)
{
	uint32_t index = id.index;
	if (index >= m_Sparse.size())
	{
		m_Sparse.resize(index + 1, INVALID_INDEX);
	}

	assert(m_Sparse[index] == INVALID_INDEX);
	m_Sparse[index] = (uint32_t)m_Dense.size();
	m_Dense.push_back(index);
	m_Components.push_back(std::move(component));
}

template<typename T>
void SparseSet<T>::Remove(EntityID id)
{
	uint32_t index = id.index;
	uint32_t denseIndex = m_Sparse[index];

	// Swap with last element to keep dense arrays packed
	uint32_t lastIndex = m_Dense.back();
	if (lastIndex != index)
	{
		m_Dense[denseIndex] = lastIndex;
		m_Components[denseIndex] = std::move(m_Components.back());
		m_Sparse[lastIndex] = denseIndex;
	}

	m_Sparse[index] = INVALID_INDEX;
	m_Dense.pop_back();
	m_Components.pop_back();
}

template<typename T>
T* SparseSet<T>::Get(EntityID id)
{
	uint32_t index = id.index;
	if (index >= m_Sparse.size() || m_Sparse[index] == INVALID_INDEX)
	{
		return nullptr;
	}

	return &m_Components[m_Sparse[index]];
}

template<typename T>
bool SparseSet<T>::Has(EntityID id) const
{
	uint32_t index = id.index;
	return index < m_Sparse.size() && m_Sparse[index] != INVALID_INDEX;
}