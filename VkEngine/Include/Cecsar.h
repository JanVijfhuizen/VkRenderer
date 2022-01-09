#pragma once
#include "SparseSet.h"

class Cecsar;

struct Entity final
{
	friend Cecsar;

	[[nodiscard]] operator bool() const;
	[[nodiscard]] bool operator ==(const Entity& other) const;
	[[nodiscard]] operator uint32_t() const;

private:
	uint32_t _identifier = 0;
	uint32_t _index;
};

class ISystem
{
public:
	explicit ISystem(Cecsar& cecsar);
	virtual void RemoveAt(uint32_t index) = 0;

protected:
	[[nodiscard]] Cecsar& GetCecsar() const;

private:
	Cecsar& _cecsar;
};

class Cecsar final : public SparseSet<Entity>
{
public:
	explicit Cecsar(size_t capacity);

	void SubscribeSystem(ISystem* system);
	void RemoveAt(uint32_t sparseIndex) override;

private:
	vi::Vector<ISystem*> _systems{32, GMEM_VOL};
};

template <typename T>
class System : public ISystem, public SparseSet<T>
{
public:
	explicit System(Cecsar& cecsar);
	virtual void RemoveAt(uint32_t index);
};

template <typename T>
System<T>::System(Cecsar& cecsar) : ISystem(cecsar), SparseSet<T>(cecsar.GetLength())
{
}

template <typename T>
void System<T>::RemoveAt(const uint32_t index)
{
	SparseSet<T>::RemoveAt(index);
}