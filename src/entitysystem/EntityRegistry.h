#pragma once

#include <entitysystem/component/Component.h>
#include <entitysystem/EntityPool.h>
#include <entitysystem/SparseSet.h>
#include <entitysystem/View.h>
#include <memory>
#include <unordered_map>

class EntityRegistry
{
public:
	EntityID CreateEntity() { return m_EntityPool.Create(); }
	void DestroyEntity(EntityID entity) { m_EntityPool.Destroy(entity); }

	template<typename T>
	void Emplace(EntityID entity, T component) { component.SetOwner(entity); GetPool<T>().Insert(entity, std::move(component)); }

	template<typename T>
	void Remove(EntityID entity) { GetPool<T>().Remove(entity); }

	template<typename T>
	T* Get(EntityID entity) { return GetPool<T>().Get(entity); }

	template<typename T>
	bool Has(EntityID e) { return GetPool<T>().Has(e); }

	template<typename... Ts>
	View<Ts...> GetView() { return View<Ts...>(GetPool<Ts>()...); }

private:
	template <typename T>
	SparseSet<T>& GetPool();

private:
	EntityPool m_EntityPool;

	std::unordered_map<ComponentTypeID, std::unique_ptr<ISparseSet>> m_Sets;
};

#include <entitysystem/EntityRegistry.inl>