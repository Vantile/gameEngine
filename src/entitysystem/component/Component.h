#pragma once

#include <cstdint>
#include <entitysystem/Entity.h>

using ComponentTypeID = uint32_t;

class Component
{
public:
	template<typename T> static ComponentTypeID GetTypeID() { static ComponentTypeID id = NextTypeID(); return id; }

	void SetOwner(const EntityID& entityID) { m_Owner = entityID; }
	const EntityID& GetOwner() const { return m_Owner; }

private:
	static ComponentTypeID NextTypeID() { static ComponentTypeID counter = 0; return counter++; }

	EntityID m_Owner;
};