#pragma once

#include <entitysystem/EntityRegistry.h>

class System
{
public:
	virtual void Update(EntityRegistry& registry) = 0;
};