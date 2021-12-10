#pragma once

namespace vi
{
	class FreeListAllocator final
	{
	public:
		explicit FreeListAllocator(size_t capacity);
		~FreeListAllocator();

		[[nodiscard]] void* Allocate(size_t size);
		void Free(void* ptr);

		[[nodiscard]] size_t GetCapacity() const;
		[[nodiscard]] void* GetData() const;

	private:
		size_t* _data;
		size_t _capacity;
		size_t* _next;
	};
}
