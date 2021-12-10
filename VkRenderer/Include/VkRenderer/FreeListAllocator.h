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

		[[nodiscard]] void* MAllocate(size_t size) const;
		void MFree(void* ptr) const;

		[[nodiscard]] size_t GetCapacity() const;

	private:
		struct Block final
		{
			size_t* data;
			size_t* next;
			Block* child = nullptr;

			explicit Block(size_t capacity);
			~Block();

			void* TryAllocate(size_t size);
			bool TryFree(void* ptr);
		};

		Block* _block;
		size_t _capacity;
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
