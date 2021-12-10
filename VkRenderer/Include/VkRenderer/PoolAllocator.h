#pragma once

namespace vi
{
	class PoolAllocator final
	{
	public:
		explicit PoolAllocator(size_t blockSize, size_t capacity);
		~PoolAllocator();

		[[nodiscard]] void* Allocate();
		void Free(void* ptr);

		[[nodiscard]] size_t GetCapacity() const;
		[[nodiscard]] size_t GetBlockSize() const;
		[[nodiscard]] size_t GetCount() const;
		[[nodiscard]] void* GetData() const;
		
	private:
		void** _data;
		size_t _blockSize;
		size_t _capacity;
		size_t _count = 0;
		void** _next;
	};

	inline PoolAllocator::PoolAllocator(const size_t blockSize, const size_t capacity) : 
		_blockSize(blockSize), _capacity(capacity)
	{
		size_t size = _blockSize / sizeof(void*);
		if (size == 0)
			size = 1;

		_data = new void*[size * _capacity];

		void** current = _data;
		for (size_t i = 0; i < _capacity - 1; ++i)
		{
			*current = &current[size];
			current = reinterpret_cast<void**>(*current);
		}

		_next = _data;
	}

	inline PoolAllocator::~PoolAllocator()
	{
		delete[] _data;
	}

	inline void* PoolAllocator::Allocate()
	{
		_count++;
		void* alloc = _next;
		_next = reinterpret_cast<void**>(*_next);
		return alloc;
	}

	inline void PoolAllocator::Free(void* ptr)
	{
		_count--;
		void** freed = reinterpret_cast<void**>(ptr);
		*freed = _next;
		_next = freed;
	}

	inline size_t PoolAllocator::GetCapacity() const
	{
		return _capacity;
	}

	inline size_t PoolAllocator::GetBlockSize() const
	{
		return _blockSize;
	}

	inline size_t PoolAllocator::GetCount() const
	{
		return _count;
	}

	inline void* PoolAllocator::GetData() const
	{
		return _data;
	}
}
