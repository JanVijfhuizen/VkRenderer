#pragma once

template <typename T>
class HashSet
{
public:
	[[nodiscard]] T* operator[] (uint32_t sparseIndex) const;

	explicit HashSet(size_t capacity, vi::FreeListAllocator& allocator = GMEM);

	virtual T& Insert(uint32_t sparseIndex, const T& value = {});
	virtual void RemoveAt(uint32_t sparseIndex);

	[[nodiscard]] size_t GetLength() const;

	[[nodiscard]] vi::Iterator<T> begin() const;
	[[nodiscard]] vi::Iterator<T> end() const;

private:
	typedef vi::KeyValue<int32_t, uint32_t> Hash;

	vi::Vector<T> _instances;
	vi::ArrayPtr<Hash> _hashes;
	vi::ArrayPtr<uint32_t> _dense;

	[[nodiscard]] int32_t ToHash(uint32_t sparseIndex) const;
	[[nodiscard]] int32_t Find(uint32_t sparseIndex) const;
};

template <typename T>
T* HashSet<T>::operator[](const uint32_t sparseIndex) const
{
	const int32_t hashIndex = Find(sparseIndex);
	return hashIndex == -1 ? nullptr : &_instances[_hashes[hashIndex].value];
}

template <typename T>
HashSet<T>::HashSet(const size_t capacity, vi::FreeListAllocator& allocator) : 
	_instances(capacity, allocator), _hashes(capacity, allocator, { -1, {} }), _dense(capacity, allocator)
{
	
}

template <typename T>
T& HashSet<T>::Insert(const uint32_t sparseIndex, const T& value)
{
	{
		// If the set already contains the value.
		T* ret = operator[](sparseIndex);
		if (ret)
			return ret;
	}

	const size_t count = _instances.GetCount() + 1;

	const auto data = _hashes.GetData();
	const size_t length = _hashes.GetLength();
	const int32_t hash = ToHash(sparseIndex);
	int32_t index = hash;

	assert(count <= length);

	Hash* sparse;
	do
	{
		sparse = &data[index];
		index = (index + 1) % length;
	} while (sparse->key != -1);

	sparse->value = _instances.GetCount();
	sparse->key = index - 1;

	_dense[index - 1] = sparseIndex;
	return _instances.Add(value);
}

template <typename T>
void HashSet<T>::RemoveAt(const uint32_t sparseIndex)
{
	int32_t hashIndex = Find(sparseIndex);
	if (hashIndex == -1)
		return;

	auto& hash = _hashes[hashIndex];

	const size_t count = _instances.GetCount() - 1;
	hashIndex = _dense[count];

	// Update other hash.
	_hashes[hashIndex].value = hash.value;

	_instances.RemoveAt(hash.value);
	_dense[hash.value] = hashIndex;

	// Reset hash.
	hash = { -1, {} };
}

template <typename T>
size_t HashSet<T>::GetLength() const
{
	return _hashes.GetLength();
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
int32_t HashSet<T>::ToHash(const uint32_t sparseIndex) const
{
	return sparseIndex % _hashes.GetLength();
}

template <typename T>
int32_t HashSet<T>::Find(const uint32_t sparseIndex) const
{
	const auto data = _hashes.GetData();
	const int32_t hash = ToHash(sparseIndex);
	int32_t index = hash;

	Hash* sparse;
	do
	{
		sparse = &data[index++];
		if (sparse->key == hash && sparseIndex == sparse->value)
			return index - 1;
	} while (sparse->key >= hash);
	return -1;
}
