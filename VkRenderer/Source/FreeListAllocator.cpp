#include "pch.h"
#include "FreeListAllocator.h"

namespace vi
{
	FreeListAllocator::FreeListAllocator(size_t capacity) : _capacity(capacity)
	{
		capacity /= sizeof(size_t);
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

	void* FreeListAllocator::MAlloc(size_t size) const
	{
		{
			const size_t allocDiff = size % sizeof(size_t);
			size += (sizeof(size_t) - allocDiff) * (allocDiff > 0);
		}

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

		const auto block = new Block(_capacity / sizeof(size_t));
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

	FreeListAllocator::Block::Block(const size_t capacity)
	{
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
		size = size / 4 + 2;

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
		const auto partition = &reinterpret_cast<size_t*>(ptr)[-2];
		const auto adjecent = &partition[partition[1] + 2];

		size_t* previous = nullptr;
		size_t* current = next;

		while (current)
		{
			auto& space = current[1];

			if (adjecent == current)
			{
				*partition = current[2];
				partition[1] += space + 2;

				if (next == current)
					next = partition;
				else
					*reinterpret_cast<size_t**>(previous) = partition;
				return true;
			}

			const auto currentAdjecent = &current[space + 2];

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
}
