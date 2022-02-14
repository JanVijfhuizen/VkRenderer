#pragma once

/// <summary>
/// Data container that avoids fragmentation and supports O(1) lookup, but requires a lot of memory.
/// </summary>
template <typename T>
class SparseSet
{
public:
	typedef vi::KeyValue<uint32_t, T> Instance;

	T& operator [](uint32_t sparseIndex) const;

	explicit SparseSet(size_t size, vi::FreeListAllocator& allocator = GMEM);

	virtual T& Insert(uint32_t sparseIndex, const T& value = {});
	virtual void RemoveAt(uint32_t sparseIndex);
	virtual void Swap(uint32_t aSparseIndex, uint32_t bSparseIndex);
	[[nodiscard]] bool Contains(uint32_t sparseIndex);

	[[nodiscard]] size_t GetLength() const;

	[[nodiscard]] vi::Iterator<Instance> begin() const;
	[[nodiscard]] vi::Iterator<Instance> end() const;

private:
	// A combination of the value and dense array.
	// This is done to make iteration, which fetches both value and sparse index, faster.
	vi::Vector<Instance> _instances;
	// Pointers to the dense/value array.
	vi::ArrayPtr<int32_t> _sparse;
};

template <typename T>
T& SparseSet<T>::operator[](const uint32_t sparseIndex) const
{
	return _instances[_sparse[sparseIndex]].value;
}

template <typename T>
SparseSet<T>::SparseSet(const size_t size, vi::FreeListAllocator& allocator) : 
	_instances(size, allocator), _sparse(size, allocator, -1)
{

}

template <typename T>
T& SparseSet<T>::Insert(const uint32_t sparseIndex, const T& value)
{
	assert(sparseIndex < _instances.GetLength());
	if (Contains(sparseIndex))
		return operator[](sparseIndex);

	auto& denseId = _sparse[sparseIndex];
	denseId = _instances.GetCount();

	auto& instance = _instances.Add({sparseIndex, value });
	return instance.value;
}

template <typename T>
void SparseSet<T>::RemoveAt(const uint32_t sparseIndex)
{
	if (!Contains(sparseIndex))
		return;

	auto& denseIndex = _sparse[sparseIndex];

	// Swap dense and values.
	_instances.RemoveAt(denseIndex);

	// Update sparse.
	const uint32_t otherSparseIndex = _instances[_instances.GetCount()].key;
	_sparse[otherSparseIndex] = denseIndex;
	denseIndex = -1;
}

template <typename T>
void SparseSet<T>::Swap(const uint32_t aSparseIndex, const uint32_t bSparseIndex)
{
	_instances.Swap(_sparse[aSparseIndex], _sparse[bSparseIndex]);
	_sparse.Swap(aSparseIndex, bSparseIndex);
}

template <typename T>
bool SparseSet<T>::Contains(const uint32_t sparseIndex)
{
	return _sparse[sparseIndex] != -1;
}

template <typename T>
size_t SparseSet<T>::GetLength() const
{
	return _instances.GetLength();
}

template <typename T>
vi::Iterator<typename SparseSet<T>::Instance> SparseSet<T>::begin() const
{
	return _instances.begin();
}

template <typename T>
vi::Iterator<typename SparseSet<T>::Instance> SparseSet<T>::end() const
{
	return _instances.end();
}
