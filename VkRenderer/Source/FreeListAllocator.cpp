#include "pch.h"
#include "FreeListAllocator.h"

namespace vi
{
	FreeListAllocator::FreeListAllocator(const size_t capacity) : _capacity(capacity)
	{
		_block = new Block(capacity);
	}

	FreeListAllocator::~FreeListAllocator()
	{
		// Delete all the blocks.
		Block* current = _block;
		while(current)
		{
			Block* next = current->child;
			delete current;
			current = next;
		}
	}

	void* FreeListAllocator::MAlloc(const size_t size) const
	{
		if (size == 0)
			return nullptr;
		assert(size <= _capacity);

		// Try out all the blocks until allocation is successful.	
		Block* previous = nullptr;
		Block* current = _block;

		while (current)
		{
			void* ptr = current->TryAllocate(size);
			if (ptr)
				return ptr;

			previous = current;
			current = current->child;
		}

		// If no block has enough space available, create a new block.
		const auto block = new Block(_capacity);
		previous->child = block;
		return block->TryAllocate(size);
	}

	bool FreeListAllocator::MFree(void* ptr) const
	{
		// Tries to free the pointer.
		Block* current = _block;
		while (current)
		{
			if (current->TryFree(ptr))
				return true;

			current = current->child;
		}

		return false;
	}

	size_t FreeListAllocator::GetCapacity() const
	{
		return _capacity;
	}

	FreeListAllocator::Block::Block(size_t capacity)
	{
		// Allocates required amount of chunks.
		capacity = ToChunkSize(capacity);
		data = new size_t[capacity];
		next = data;

		// Set next pointer to null, since there is only one range so far.
		*reinterpret_cast<void**>(data) = nullptr;
		// Set the size. -2 due to the fact that chunk 1 and 2 are reserved for the next and size values.
		next[1] = capacity - 2;
	}

	FreeListAllocator::Block::~Block()
	{
		delete[] data;
	}

	void* FreeListAllocator::Block::TryAllocate(size_t size)
	{
		size = ToChunkSize(size);
		size_t* current = next;

		// Iterate over available memory ranges.
		while (current)
		{
			auto& space = current[1];
			const auto next = reinterpret_cast<size_t*>(*current);

			// If there is not enough space.
			if (space < size)
			{
				current = next;
				continue;
			}

			const size_t diff = space - size;

			// If there is still space left over, partition this range into two parts.
			if (diff > 1)
			{
				space = size;

				const auto partitioned = reinterpret_cast<size_t*>(&current[size + 2]);
				*reinterpret_cast<size_t**>(current) = partitioned;
				*reinterpret_cast<size_t**>(partitioned) = next;

				partitioned[1] = diff - 2;
			}

			// If the first memory range was used.
			if (this->next == current)
				this->next = reinterpret_cast<size_t*>(*current);
			return &current[2];
		}

		return nullptr;
	}

	bool FreeListAllocator::Block::TryFree(void* ptr)
	{
		if (!ptr)
			return true;

		const auto partition = reinterpret_cast<size_t*>(ptr) - 2;
		// The memory range in front of this range (doesn't matter if it's out of bounds).
		const auto adjecent = partition + partition[1] + 2;

		size_t* previous = nullptr;
		size_t* current = next;

		// Iterates over the memory ranges.
		while (current)
		{
			auto& space = current[1];

			// If it's the memory range in front.
			if (adjecent == current)
			{
				*partition = *current;
				partition[1] += space + 2;

				if (!previous)
					next = partition;
				else
					*reinterpret_cast<size_t**>(previous) = partition;
				return true;
			}

			// If it's the memory range behind.
			const auto currentAdjecent = current + space + 2;

			if (currentAdjecent == partition)
			{
				space += partition[1] + 2;
				return true;
			}

			const auto next = reinterpret_cast<size_t*>(*current);
			previous = current;
			current = next;
		}

		return false;
	}

	size_t FreeListAllocator::Block::ToChunkSize(const size_t size)
	{
		return size / sizeof(size_t) + 2 + (size % sizeof(size_t) != 0);
	}
}
