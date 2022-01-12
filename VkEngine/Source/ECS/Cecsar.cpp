#include "pch.h"
#include "eCS/Cecsar.h"

namespace ce
{
	Entity::operator bool() const
	{
		return _identifier > 0;
	}

	bool Entity::operator==(const Entity& other) const
	{
		return _identifier == other._identifier;
	}

	Entity::operator unsigned() const
	{
		return _index;
	}

	ISystem::ISystem(Cecsar& cecsar) : _cecsar(cecsar)
	{

	}

	Cecsar& ISystem::GetCecsar() const
	{
		return _cecsar;
	}

	Cecsar::Cecsar(const size_t capacity) : SparseSet<Entity>(capacity, GMEM)
	{
	}

	void Cecsar::SubscribeSystem(ISystem* system)
	{
		_systems.Add(system);
	}

	Entity& Cecsar::Insert(const uint32_t sparseIndex, const Entity& value)
	{
		auto& instance = SparseSet<Entity>::Insert(sparseIndex, value);
		instance._identifier = _next++;
		instance._index = sparseIndex;
		return instance;
	}

	void Cecsar::RemoveAt(const uint32_t sparseIndex)
	{
		for (auto& system : _systems)
			system->RemoveAt(sparseIndex);
		SparseSet<Entity>::RemoveAt(sparseIndex);
	}
}