﻿#pragma once

/// <summary>
/// Data container that avoids fragmentation and supports O(1)-O(N) lookup.
/// </summary>
template <typename T>
class HashSet
{
public:
	typedef vi::KeyValue<uint32_t, T> Instance;

	[[nodiscard]] T& operator[] (uint32_t sparseIndex);

	explicit HashSet(size_t size, vi::FreeListAllocator& allocator = GMEM);

	virtual T& Insert(uint32_t sparseIndex, const T& value = {});
	virtual void RemoveAt(uint32_t sparseIndex);

	[[nodiscard]] size_t GetLength() const;

	[[nodiscard]] vi::Iterator<Instance> begin() const;
	[[nodiscard]] vi::Iterator<Instance> end() const;

private:
	struct Hashable final
	{
		uint32_t value;
		uint32_t denseIndex;

		[[nodiscard]] size_t operator%(size_t mod) const;
		[[nodiscard]] bool operator==(const Hashable& other) const;
	};

	vi::Vector<Instance> _instances;
	vi::HashMap<Hashable> _hashMap;
};

template <typename T>
T& HashSet<T>::operator[](const uint32_t sparseIndex)
{
	Hashable* hashable = _hashMap.Find({ sparseIndex, 0 });
	assert(hashable);
	return _instances[hashable->denseIndex].value;
}

template <typename T>
HashSet<T>::HashSet(const size_t size, vi::FreeListAllocator& allocator) :
	_instances(size, allocator), _hashMap(size, allocator)
{
	
}

template <typename T>
T& HashSet<T>::Insert(const uint32_t sparseIndex, const T& value)
{
	assert(_instances.GetCount() < _instances.GetLength());
	const uint32_t count = _instances.GetCount();
	_hashMap.Insert({ sparseIndex, count });
	return _instances.Add({ sparseIndex, value }).value;
}

template <typename T>
void HashSet<T>::RemoveAt(const uint32_t sparseIndex)
{
	Hashable* hashable = _hashMap.Find({ sparseIndex, 0 });
	if (!hashable)
		return;

	const uint32_t denseIndex = hashable->denseIndex;
	_instances.RemoveAt(denseIndex);
	_hashMap.Remove({ sparseIndex, 0});

	// Decrement pointing index if it's higher than the one removed.
	for (auto& keyPair : _hashMap)
	{
		auto& keypairDenseIndex = keyPair.value.denseIndex;
		keypairDenseIndex -= keypairDenseIndex > denseIndex ? 1 : 0;
	}
}

template <typename T>
size_t HashSet<T>::GetLength() const
{
	return _hashMap.GetLength();
}

template <typename T>
vi::Iterator<typename HashSet<T>::Instance> HashSet<T>::begin() const
{
	return _instances.begin();
}

template <typename T>
vi::Iterator<typename HashSet<T>::Instance> HashSet<T>::end() const
{
	return _instances.end();
}

template <typename T>
size_t HashSet<T>::Hashable::operator%(const size_t mod) const
{
	return value % mod;
}

template <typename T>
bool HashSet<T>::Hashable::operator==(const Hashable& other) const
{
	return value == other.value;
}
