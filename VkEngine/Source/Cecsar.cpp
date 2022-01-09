#include "pch.h"
#include "Cecsar.h"

Cecsar::ISystem::ISystem(Cecsar& cecsar) : _cecsar(cecsar)
{

}

Cecsar* Cecsar::ISystem::GetCecsar() const
{
	return &_cecsar;
}

Cecsar::Entity::operator bool() const
{
	return _identifier > 0;
}

bool Cecsar::Entity::operator==(const Entity& other) const
{
	return _identifier == other._identifier;
}

Cecsar::Entity::operator uint32_t() const
{
	return _index;
}

void Cecsar::SubscribeSystem(ISystem* system)
{
	_systems.Add(system);
}
