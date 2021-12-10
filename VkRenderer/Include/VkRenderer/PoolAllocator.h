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
}
