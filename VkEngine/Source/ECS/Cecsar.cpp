#include "pch.h"
#include "ECS/Cecsar.h"

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

	Entity::operator uint16_t() const
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

	Cecsar::Cecsar(const uint16_t capacity)
	{
		_instances = vi::ArrayPtr<uint16_t>{capacity, GMEM};
		_open = vi::BinTree<uint16_t>{capacity, GMEM};
	}

	void Cecsar::SubscribeSystem(ISystem* system)
	{
		_systems.Add(system);
	}

	Entity Cecsar::Add()
	{
		assert(_count < _instances.GetLength());

		uint16_t index = _count;
		if(!_open.IsEmpty())
			index = _open.Pop();

		// Create a new entity with target index and a unique identifier.
		Entity entity{};
		entity._index = index;
		entity._identifier = _numCreatedEntities++;
		_count++;
		return entity;
	}

	void Cecsar::RemoveAt(const uint16_t sparseIndex)
	{
		_count--;
		// Remove all attached components.
		for (auto& system : _systems)
			system->RemoveAt(sparseIndex);
		_open.Push({ static_cast<uint16_t>(sparseIndex), sparseIndex });
	}

	uint16_t Cecsar::GetCount() const
	{
		return _count;
	}

	size_t Cecsar::GetCapacity() const
	{
		return _instances.GetLength();
	}
}
