#pragma once

namespace vi
{
	class FreeListAllocator final
	{
	public:
		explicit FreeListAllocator(size_t capacity);
		~FreeListAllocator();

		template <typename T, typename ...Args>
		[[nodiscard]] T* Allocate(Args... args);
		template <typename T>
		void Free(T* ptr);

		[[nodiscard]] void* MAllocate(size_t size);
		void MFree(void* ptr);

		[[nodiscard]] size_t GetCapacity() const;
		[[nodiscard]] void* GetData() const;

	private:
		size_t* _data;
		size_t _capacity;
		size_t* _next;
	};

	template <typename T, typename ... Args>
	T* FreeListAllocator::Allocate(Args... args)
	{
		const auto ptr = reinterpret_cast<T*>(MAllocate(sizeof(T)));
		new (ptr) T(args...);
		return ptr;
	}

	template <typename T>
	void FreeListAllocator::Free(T* ptr)
	{
		ptr->~T();
		MFree(ptr);
	}
}
