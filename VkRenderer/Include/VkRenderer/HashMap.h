#pragma once

namespace vi
{
	/// <summary>
	/// Container used to quickly validate if a value is present in the container.
	/// </summary>
	template <typename T>
	class HashMap final : public ArrayPtr<KeyValue<int32_t, T>>
	{
	public:
		HashMap(size_t size, FreeListAllocator& allocator);

		// Add a new value in the container.
		void Insert(const T& value);
		/// <returns>A pointer to the value if present, otherwise returns a nullptr.</returns>
		T* Contains(const T& value);
		// Remove object if present in the container.
		void Remove(const T& value);

		[[nodiscard]] size_t GetCount() const;
		[[nodiscard]] bool IsEmpty() const;

		/// <returns>Target object if present, otherwise returns a nullptr.</returns>
		[[nodiscard]] T* Find(const T& value);

	private:
		typedef KeyValue<int32_t, T> Node;

		size_t _count = 0;

		[[nodiscard]] Node* FindNode(const T& value, uint32_t* outIndex = nullptr);
		[[nodiscard]] int32_t ToHash(const T& value) const;
	};

	template <typename T>
	HashMap<T>::HashMap(const size_t size, FreeListAllocator& allocator) : ArrayPtr<Node>(size, allocator, { -1, {} })
	{
		
	}

	template <typename T>
	void HashMap<T>::Insert(const T& value)
	{
		// Don't add it if it already contains something at that spot.
		if (Contains(value))
			return;

		_count++;

		const auto data = ArrayPtr<Node>::GetData();
		const size_t length = ArrayPtr<Node>::GetLength();
		const int32_t hash = ToHash(value);
		int32_t index = hash;

		assert(_count <= length);

		// Try to fit the object as close to the original hash position as possible.
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
		const auto node = FindNode(value);
		return node ? &node->value : nullptr;
	}

	template <typename T>
	void HashMap<T>::Remove(const T& value)
	{
		uint32_t index;
		const auto node = FindNode(value, &index);
		// If the node is not found, ignore this request.
		if (!node)
			return;

		const auto data = ArrayPtr<Node>::GetData();
		const size_t length = ArrayPtr<Node>::GetLength();

		// Find the node.
		Node* neighbour = node;
		while (node->key == neighbour->key)
		{
			index = (index + 1) % length;
			neighbour = &data[index];
		} 

		// Clean the node and replace it with a potential candidate fit for that spot.
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
	T* HashMap<T>::Find(const T& value)
	{
		const auto node = FindNode(value);
		return node ? &node->value : nullptr;
	}

	template <typename T>
	typename HashMap<T>::Node* HashMap<T>::FindNode(const T& value, uint32_t* outIndex)
	{
		const auto data = ArrayPtr<Node>::GetData();
		const int32_t hash = ToHash(value);
		int32_t index = hash;

		// Try and find the node based on the hash and the value.
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
