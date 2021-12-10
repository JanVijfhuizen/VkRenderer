#include "pch.h"
#include "FreeListAllocator.h"

namespace vi
{
	FreeListAllocator::FreeListAllocator(size_t capacity) : _capacity(capacity)
	{
		capacity /= sizeof(size_t);

		_data = new size_t[capacity];
		_next = _data;

		*reinterpret_cast<void**>(_data) = nullptr;
		_next[1] = capacity - 2;
	}

	FreeListAllocator::~FreeListAllocator()
	{
		delete[] _data;
	}

	void* FreeListAllocator::MAllocate(size_t size)
	{
		size = size / 4 + 2;

		size_t* current = _next;

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

			if (_next == current)
				_next = reinterpret_cast<size_t*>(*current);
			return &current[2];
		}

		return nullptr;
	}

	void FreeListAllocator::MFree(void* ptr)
	{
		const auto partition = &reinterpret_cast<size_t*>(ptr)[-2];
		const auto adjecent = &partition[partition[1] + 2];

		size_t* previous = nullptr;
		size_t* current = _next;

		while (current)
		{
			auto& space = current[1];

			if (adjecent == current)
			{
				*partition = current[2];
				partition[1] += space + 2;

				if (_next == current)
					_next = partition;
				else
					*reinterpret_cast<size_t**>(previous) = partition;
				return;
			}

			const auto currentAdjecent = &current[space + 2];

			if (currentAdjecent == partition)
			{
				space += partition[1] + 2;
				return;
			}

			const auto next = reinterpret_cast<size_t*>(*current);
			previous = current;
			current = next;
		}
	}

	size_t FreeListAllocator::GetCapacity() const
	{
		return _capacity;
	}

	void* FreeListAllocator::GetData() const
	{
		return _data;
	}
}