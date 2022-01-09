#pragma once

template <typename T>
class SparseSet final
{
public:
	struct Instance final
	{
		T value;
		uint32_t sparseIndex;
	};

	T& operator [](uint32_t sparseIndex) const;

	explicit SparseSet(size_t capacity, vi::FreeListAllocator& allocator = GMEM);

	T& Insert(uint32_t sparseIndex, const T& value = {});
	void RemoveAt(uint32_t sparseIndex);
	[[nodiscard]] bool Contains(uint32_t sparseIndex);

	[[nodiscard]] vi::Iterator<Instance> begin() const;
	[[nodiscard]] vi::Iterator<Instance> end() const;

private:
	vi::Vector<Instance> _instances;
	vi::ArrayPtr<int32_t> _sparse;
	vi::ArrayPtr<uint32_t> _dense;
};

template <typename T>
T& SparseSet<T>::operator[](const uint32_t sparseIndex) const
{
	return _instances[_sparse[sparseIndex]].value;
}

template <typename T>
SparseSet<T>::SparseSet(const size_t capacity, vi::FreeListAllocator& allocator) : 
	_instances(capacity, allocator), _sparse(capacity, allocator, -1), _dense(capacity, allocator, -1)
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
	_dense[denseId] = sparseIndex;

	auto& instance = _instances.Add({value, sparseIndex});
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
	const uint32_t otherSparseIndex = _dense[denseIndex] = _dense[_instances.GetCount()];

	// Update sparse.
	_sparse[otherSparseIndex] = denseIndex;
	denseIndex = -1;
}

template <typename T>
bool SparseSet<T>::Contains(const uint32_t sparseIndex)
{
	return _sparse[sparseIndex] != -1;
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
