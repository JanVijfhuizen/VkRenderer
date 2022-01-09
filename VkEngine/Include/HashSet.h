#pragma once

/// <summary>
/// Data container that avoids fragmentation and supports O(1)-O(N) lookup.<br>
/// In the ECS, This set is most useful when working with a small amount of component instances.
/// </summary>
template <typename T>
class HashSet
{
public:
	[[nodiscard]] T& operator[] (uint32_t sparseIndex);

	explicit HashSet(size_t size, vi::FreeListAllocator& allocator = GMEM);

	virtual T& Insert(uint32_t sparseIndex, const T& value = {});
	virtual void RemoveAt(uint32_t sparseIndex);

	[[nodiscard]] size_t GetLength() const;

	[[nodiscard]] vi::Iterator<T> begin() const;
	[[nodiscard]] vi::Iterator<T> end() const;

private:
	struct Hashable final
	{
		uint32_t value;
		uint32_t denseIndex;

		[[nodiscard]] size_t operator%(size_t mod) const;
		[[nodiscard]] bool operator==(const Hashable& other) const;
	};

	vi::Vector<T> _instances;
	vi::HashMap<Hashable> _hashMap;
	vi::ArrayPtr<uint32_t> _dense;
};

template <typename T>
T& HashSet<T>::operator[](const uint32_t sparseIndex)
{
	Hashable* hashable = _hashMap.FindNode({ sparseIndex, -1 });
	assert(hashable);
	return _instances[hashable->denseIndex];
}

template <typename T>
HashSet<T>::HashSet(const size_t size, vi::FreeListAllocator& allocator) :
	_instances(size, allocator), _hashMap(size, allocator), _dense(size, allocator)
{
	
}

template <typename T>
T& HashSet<T>::Insert(const uint32_t sparseIndex, const T& value)
{
	assert(_instances.GetCount() < _instances.GetLength());
	const uint32_t count = _instances.GetCount();
	_hashMap.Insert({ sparseIndex, count });
	_dense[count] = sparseIndex;
	return _instances.Add(value);
}

template <typename T>
void HashSet<T>::RemoveAt(const uint32_t sparseIndex)
{
	Hashable* hashable = _hashMap.Find({ sparseIndex, 0 });
	if (!hashable)
		return;

	const uint32_t denseIndex = hashable->denseIndex;
	_instances.RemoveAt(denseIndex);
	_dense[denseIndex] = _dense[_instances.GetCount()];
	_hashMap.Remove({ sparseIndex , 0});

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
vi::Iterator<T> HashSet<T>::begin() const
{
	return _instances.begin();
}

template <typename T>
vi::Iterator<T> HashSet<T>::end() const
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
