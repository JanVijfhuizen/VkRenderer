#pragma once

namespace vi
{
	#ifndef STACK_ALLOCATOR_ALLOC_SIZE
	#define STACK_ALLOCATOR_ALLOC_SIZE 1000
	#endif

	class StackAllocator final
	{
	public:
		StackAllocator();
		~StackAllocator();

		template <typename T>
		T* Alloc(uint64_t count = 1);
		void Pop();

	private:
		struct Block final
		{
			size_t data[STACK_ALLOCATOR_ALLOC_SIZE];
			Block* previous = nullptr;
			size_t offset = 0;
		};

		Block* _current;
	};

	template <typename T>
	T* StackAllocator::Alloc(const uint64_t count)
	{
		const size_t size = sizeof(T) * count + 1;
		assert(size <= STACK_ALLOCATOR_ALLOC_SIZE);

		if (size > STACK_ALLOCATOR_ALLOC_SIZE - _current->offset)
		{
			Block* newBlock = new Block;
			_current->previous = newBlock;
			_current = newBlock;
		}

		const auto ptr = &_current->data[_current->offset];
		ptr[size - 1] = size;
		const auto begin = reinterpret_cast<T*>(ptr);
		_current->offset += size;
		return begin;
	}
}