#pragma once

#include <cstdint>

struct EntityID
{
	uint32_t index : 24;
	uint8_t generation : 8;
};