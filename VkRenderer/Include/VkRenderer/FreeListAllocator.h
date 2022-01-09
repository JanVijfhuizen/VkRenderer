#pragma once
#include <cstdint>

namespace vi
{
	/// <summary>
	/// An allocator which offers free allocations and deallocations at will, at the cost of possible fragmentation.
	/// </summary>
	class FreeListAllocator final
	{
	public:
		/// <param name="capacity">Capacity per block. Allocator will create multiple blocks if memory runs out.</param>
		explicit FreeListAllocator(size_t capacity);
		~FreeListAllocator();

		/// <summary>
		/// Allocates object of type T. Allocator is called.
		/// </summary>
		/// <param name="args">Arguments which are passed to the constructor.</param>
		template <typename T, typename ...Args>
		[[nodiscard]] T* New(Args&... args);
		/// <summary>
		/// Deallocates object of type T. Destructor is called.
		/// </summary>
		template <typename T>
		void Delete(T* ptr);

		/// <summary> Manually allocate a block of memory. </summary>
		[[nodiscard]] void* MAlloc(size_t size) const;
		/// <summary> Manually frees a block of memory.</summary>
		/// <returns>Whether or not the pointer was part of this allocator.</returns>
		bool MFree(void* ptr) const;
		/// <returns>Capacity per block.</returns>
		[[nodiscard]] size_t GetCapacity() const;

	private:
		/// <summary>
		/// Object which holds a range of data.<br>
		/// A range of size N is stored like this: [next][size][0][1][2][...][N].
		/// </summary>
		struct Block final
		{
			// Memory this block manages.
			size_t* data;
			// Next/first memory slot to check when allocating.
			size_t* next;
			// Linked list.
			Block* child = nullptr;

			explicit Block(size_t capacity);
			~Block();

			/// <returns>Pointer to memory range if successful, nullptr if not.</returns>
			[[nodiscard]] void* TryAllocate(size_t size);
			/// <summary>Tries to free a range of memory in this block.</summary>
			/// <returns>Whether or not the pointer was part of this memory block.</returns>
			bool TryFree(void* ptr);
			/// <summary>Calculates how many size_t chunks are required for this memory range.</summary>
			[[nodiscard]] static size_t ToChunkSize(size_t size);
		};

		// Linked list of blocks.
		Block* _block;
		// Capacity per block.
		size_t _capacity;
	};

	template <typename T, typename ... Args>
	T* FreeListAllocator::New(Args&... args)
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
