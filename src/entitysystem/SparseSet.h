#pragma once

#include <cstdint>
#include <entitysystem/Entity.h>
#include <span>
#include <vector>

class ISparseSet
{

};

template <typename T>
class SparseSet : public ISparseSet
{
public:
	void Insert(EntityID id, T component);
	void Remove(EntityID id);
	T* Get(EntityID id);
	bool Has(EntityID id) const;

	std::span<T> Components() { return m_Components; }
	std::span<uint32_t> Entities() { return m_Dense; }
	size_t Size() const { return m_Dense.size(); }
private:
	static constexpr uint32_t INVALID_INDEX = UINT32_MAX;

	std::vector<uint32_t> m_Sparse;
	std::vector<uint32_t> m_Dense;
	std::vector<T> m_Components;
};

#include <entitysystem/SparseSet.inl>