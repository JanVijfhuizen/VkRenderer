#include "pch.h"
#include "PoolAllocator.h"

namespace vi
{
	PoolAllocator::PoolAllocator(const size_t blockSize, const size_t capacity) :
		_blockSize(blockSize), _capacity(capacity)
	{
		size_t size = _blockSize / sizeof(void*);
		if (size == 0)
			size = 1;

		_data = new void* [size * _capacity];

		void** current = _data;
		for (size_t i = 0; i < _capacity - 1; ++i)
		{
			*current = &current[size];
			current = reinterpret_cast<void**>(*current);
		}

		_next = _data;
	}

	PoolAllocator::~PoolAllocator()
	{
		delete[] _data;
	}

	void* PoolAllocator::Allocate()
	{
		_count++;
		void* alloc = _next;
		_next = reinterpret_cast<void**>(*_next);
		return alloc;
	}

	void PoolAllocator::Free(void* ptr)
	{
		_count--;
		void** freed = reinterpret_cast<void**>(ptr);
		*freed = _next;
		_next = freed;
	}

	size_t PoolAllocator::GetCapacity() const
	{
		return _capacity;
	}

	size_t PoolAllocator::GetBlockSize() const
	{
		return _blockSize;
	}

	size_t PoolAllocator::GetCount() const
	{
		return _count;
	}

	void* PoolAllocator::GetData() const
	{
		return _data;
	}
}