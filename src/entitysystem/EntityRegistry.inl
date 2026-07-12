template <typename T>
SparseSet<T>& EntityRegistry::GetPool()
{
	ComponentTypeID id = Component::GetTypeID<T>();
	auto it = m_Sets.find(id);
	if (it == m_Sets.end())
	{
		auto [inserted, _] = m_Sets.emplace(id, std::make_unique<SparseSet<T>>());
		return *static_cast<SparseSet<T>*>(inserted->second.get());
	}

	return *static_cast<SparseSet<T>*>(it->second.get());
}