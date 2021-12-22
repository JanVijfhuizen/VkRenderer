#pragma once

namespace vi
{
	class FreeListAllocator final
	{
	public:
		explicit FreeListAllocator(size_t capacity);
		~FreeListAllocator();

		template <typename T, typename ...Args>
		[[nodiscard]] T* New(Args... args);
		template <typename T>
		void Delete(T* ptr);

		[[nodiscard]] void* MAlloc(size_t size) const;
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

			[[nodiscard]] void* TryAllocate(size_t size);
			bool TryFree(void* ptr);

			[[nodiscard]] static size_t ToChunkSize(size_t size);
		};

		Block* _block;
		size_t _capacity;
	};

	template <typename T, typename ... Args>
	T* FreeListAllocator::New(Args... args)
	{
		const auto ptr = reinterpret_cast<T*>(MAlloc(sizeof(T)));
		new (ptr) T(args...);
		return ptr;
	}

	template <typename T>
	void FreeListAllocator::Delete(T* ptr)
	{
		ptr->~T();
		MFree(ptr);
	}
}
