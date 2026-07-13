#pragma once

#include <cstdint>
#include <xhash>

struct EntityID
{
	uint32_t index : 24;
	uint8_t generation : 8;
};

template<>
struct std::hash<EntityID>
{
	size_t operator()(const EntityID& id) const noexcept
	{
		uint32_t packed = (id.generation << 24) | id.index;
		return std::hash<uint32_t>{}(packed);
	}
};

inline bool operator==(const EntityID& a, const EntityID& b) noexcept
{
	return a.index == b.index && a.generation == b.generation;
}