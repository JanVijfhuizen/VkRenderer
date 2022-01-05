#pragma once
#include "KeyValue.h"

namespace vi
{
	template <typename T>
	class BinTree final : public ArrayPtr<KeyValue<T, int32_t>>
	{
	public:
		typedef KeyValue<T, int32_t> Node;

		void Push(const Node& node);
		[[nodiscard]] T Peek();
		T Pop();

	private:
		size_t _count = 0;
	};

	template <typename T>
	void BinTree<T>::Push(const Node& node)
	{
		_count++;
		assert(_count <= ArrayPtr<Node>::GetLength());
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
	}
}
