#pragma once
#include "KeyValue.h"

namespace vi
{
	template <typename T>
	class BinTree final : public ArrayPtr<KeyValue<T, int32_t>>
	{
	public:
		typedef KeyValue<T, int32_t> Node;

		explicit BinTree(size_t size, FreeListAllocator& allocator);

		void Push(const Node& node);
		[[nodiscard]] T Peek();
		T Pop();
		void Clear();

		[[nodiscard]] Iterator<KeyValue<T, int32_t>> end() const override;
	private:
		size_t _count = 0;

		void HeapifyBottomToTop(uint32_t index);
		void HeapifyTopToBottom(uint32_t index);
		void Swap(uint32_t a, uint32_t b);
	};

	template <typename T>
	BinTree<T>::BinTree(const size_t size, FreeListAllocator& allocator) : ArrayPtr<Node>(size, allocator)
	{
	}

	template <typename T>
	void BinTree<T>::Push(const Node& node)
	{
		assert(_count < ArrayPtr<Node>::GetLength());
		const auto data = ArrayPtr<Node>::GetData();

		data[_count] = node;
		HeapifyBottomToTop(_count);

		_count++;
	}

	template <typename T>
	T BinTree<T>::Peek()
	{
		assert(_count > 0);
		return ArrayPtr<Node>::GetData()[0];
	}

	template <typename T>
	T BinTree<T>::Pop()
	{
		assert(_count > 0);

		const auto data = ArrayPtr<Node>::GetData();
		T value = data[0].value;
		data[0] = data[--_count];

		HeapifyTopToBottom(0);
		return value;
	}

	template <typename T>
	void BinTree<T>::Clear()
	{
		_count = 0;
	}

	template <typename T>
	Iterator<KeyValue<T, int32_t>> BinTree<T>::end() const
	{
		Iterator<Node> it{};
		it.begin = ArrayPtr<Node>::GetData();
		it.length = _count;
		it.index = _count;

		return it;
	}

	template <typename T>
	void BinTree<T>::HeapifyBottomToTop(const uint32_t index)
	{
		// Tree root found.
		if (index == 0)
			return;

		const auto data = ArrayPtr<Node>::GetData();
		uint32_t parentIndex = index / 2;
		
		// If current is smaller than the parent, swap and continue.
		if (data[index].key < data[parentIndex].key)
		{
			Swap(index, parentIndex);
			HeapifyBottomToTop(parentIndex);
		}
	}

	template <typename T>
	void BinTree<T>::HeapifyTopToBottom(const uint32_t index)
	{
		const uint32_t left = index * 2;
		const uint32_t right = index * 2 + 1;

		// If no more nodes remain on the left side.
		if (_count < left)
			return;

		const auto data = ArrayPtr<Node>::GetData();
		// Is the left node smaller than index.
		const bool lDiff = data[index].key > data[left].key;
		// Is the right node smaller than index.
		const bool rDiff = _count > left ? data[index].key > data[right].key : false;
		// Is left smaller than right.
		const bool dir = rDiff ? data[left].key < data[right].key : true;

		if(lDiff || rDiff)
		{
			const uint32_t newIndex = left + dir;
			Swap(newIndex, index);
			HeapifyTopToBottom(newIndex);
		}
	}

	template <typename T>
	void BinTree<T>::Swap(const uint32_t a, const uint32_t b)
	{
		const auto data = ArrayPtr<Node>::GetData();
		Node temp = data[a];
		data[a] = data[b];
		data[b] = temp;
	}
}
