#pragma once

namespace vi
{
	/// <summary>
	/// Container used to quickly validate the presence of values.
	/// </summary>
	template <typename T>
	class HashMap final : public ArrayPtr<KeyValue<int32_t, T>>
	{
	public:
		HashMap(size_t size, FreeListAllocator& allocator);

		void Insert(const T& value);
		/// <returns>A pointer to the value if present, otherwise returns a nullptr.</returns>
		T* Contains(const T& value);
		void Erase(const T& value);

		[[nodiscard]] size_t GetCount() const;
		[[nodiscard]] bool IsEmpty() const;

	private:
		typedef KeyValue<int32_t, T> Node;

		size_t _count = 0;

		[[nodiscard]] Node* Find(const T& value, uint32_t* outIndex = nullptr);
		[[nodiscard]] int32_t ToHash(const T& value) const;
	};

	template <typename T>
	HashMap<T>::HashMap(const size_t size, FreeListAllocator& allocator) : ArrayPtr<Node>(size, allocator, { -1, {} })
	{
		
	}

	template <typename T>
	void HashMap<T>::Insert(const T& value)
	{
		if (Contains(value))
			return;

		_count++;

		const auto data = ArrayPtr<Node>::GetData();
		const size_t length = ArrayPtr<Node>::GetLength();
		const int32_t hash = ToHash(value);
		int32_t index = hash;

		assert(_count <= length);

		Node* node;
		do
		{
			node = &data[index];
			index = (index + 1) % length;
		} while (node->key != -1);

		node->value = value;
		node->key = hash;
	}

	template <typename T>
	T* HashMap<T>::Contains(const T& value)
	{
		const auto node = Find(value);
		return node ? &node->value : nullptr;
	}

	template <typename T>
	void HashMap<T>::Erase(const T& value)
	{
		uint32_t index;
		const auto node = Find(value, &index);
		if (!node)
			return;

		const auto data = ArrayPtr<Node>::GetData();
		const size_t length = ArrayPtr<Node>::GetLength();

		Node* neighbour = node;
		while (node->key == neighbour->key)
		{
			index = (index + 1) % length;
			neighbour = &data[index];
		} 

		Node* other = &data[(length + index - 1) % length];
		*node = *other;
		*other = { -1, {} };
		_count--;
	}

	template <typename T>
	size_t HashMap<T>::GetCount() const
	{
		return _count;
	}

	template <typename T>
	bool HashMap<T>::IsEmpty() const
	{
		return GetCount() == 0;
	}

	template <typename T>
	typename HashMap<T>::Node* HashMap<T>::Find(const T& value, uint32_t* outIndex)
	{
		const auto data = ArrayPtr<Node>::GetData();
		const int32_t hash = ToHash(value);
		int32_t index = hash;

		Node* node;
		do
		{
			node = &data[index++];
			if (node->key == hash && value == node->value)
			{
				if (outIndex)
					*outIndex = index - 1;
				return node;
			}
		} while (node->key >= hash);
		return nullptr;
	}

	template <typename T>
	int32_t HashMap<T>::ToHash(const T& value) const
	{
		return value % ArrayPtr<Node>::GetLength();
	}
}
