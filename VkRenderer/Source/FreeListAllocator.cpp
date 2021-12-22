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

		const auto block = new Block(_capacity);
		previous->child = block;
		return block->TryAllocate(size);
	}

	void FreeListAllocator::MFree(void* ptr) const
	{
		Block* current = _block;
		while (current)
		{
			if (current->TryFree(ptr))
				return;

			current = current->child;
		}
	}

	size_t FreeListAllocator::GetCapacity() const
	{
		return _capacity;
	}

	FreeListAllocator::Block::Block(size_t capacity)
	{
		capacity = ToChunkSize(capacity);
		data = new size_t[capacity];
		next = data;

		*reinterpret_cast<void**>(data) = nullptr;
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

		while (current)
		{
			auto& space = current[1];
			const auto next = reinterpret_cast<size_t*>(*current);

			if (space < size)
			{
				current = next;
				continue;
			}

			const size_t diff = space - size;

			if (diff > 1)
			{
				space = size;

				const auto partitioned = reinterpret_cast<size_t*>(&current[size + 2]);
				*reinterpret_cast<size_t**>(current) = partitioned;
				*reinterpret_cast<size_t**>(partitioned) = next;

				partitioned[1] = diff - 2;
			}

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
		const auto adjecent = partition + partition[1] + 2;

		size_t* previous = nullptr;
		size_t* current = next;

		while (current)
		{
			auto& space = current[1];

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
		return size / 4 + 2 + (size % sizeof(size_t) != 0);
	}
}
