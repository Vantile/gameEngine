#pragma once

#include <cstdint>

using ComponentTypeID = uint32_t;

class Component
{
public:
	template<typename T> static ComponentTypeID GetTypeID() { static ComponentTypeID id = NextTypeID(); return id; }

private:
	static ComponentTypeID NextTypeID() { static ComponentTypeID counter = 0; return counter++; }
};