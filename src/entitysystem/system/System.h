#pragma once

#include <engine/FrameData.h>
#include <entitysystem/EntityRegistry.h>

class System
{
public:
	virtual void Update(EntityRegistry& registry, FrameData& frameData) = 0;
};