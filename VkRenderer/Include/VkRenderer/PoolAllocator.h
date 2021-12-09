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
		[[nodiscard]] void** GetData() const;
		
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
		_data = new void*[blockSize * capacity];
		for (size_t i = 0; i < capacity - 1; ++i)
			_data[i * blockSize] = &_data[(i + 1) * _blockSize];
		_next = _data;
	}

	inline PoolAllocator::~PoolAllocator()
	{
		delete[] _data;
	}

	inline void* PoolAllocator::Allocate()
	{
		_count++;
		void** alloc = _next;
		_next = reinterpret_cast<void**>(*alloc);
		return reinterpret_cast<void*>(alloc);
	}

	inline void PoolAllocator::Free(void* ptr)
	{
		_count--;
		void** r = reinterpret_cast<void**>(ptr);
		*r = _next;
		_next = r;
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

	inline void** PoolAllocator::GetData() const
	{
		return _data;
	}
}
