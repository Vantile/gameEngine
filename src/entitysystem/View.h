#pragma once

#include <entitysystem/SparseSet.h>
#include <tuple>

template<typename... Ts>
class View
{
public:
	View(SparseSet<Ts>&... sets) : m_Sets(&sets...) {}

	template<typename Func>
	void Each(Func&& func)
	{
		// Iterate smallest set, check all others
		auto* primary = std::get<0>(m_Sets); // TODO: Get smallest pool, not just the first one

		for (size_t i = 0; i < primary->Size(); ++i)
		{
			EntityID id = { primary->Entities()[i], 0 }; // TODO: Modify whatever this is

			bool valid = (std::get<SparseSet<Ts>*>(m_Sets)->Has(id) && ...);
			if (!valid)
			{
				continue;
			}

			func(*std::get<SparseSet<Ts>*>(m_Sets)->Get(id)...);
		}
	}

private:
	SparseSet<void>* SmallestSet() { /* Return set with minimum Size() */ };

private:
	std::tuple<SparseSet<Ts>*...> m_Sets;
};