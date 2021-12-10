#pragma once

namespace vi
{
	class FreeListAllocator final
	{
	public:
		explicit FreeListAllocator(size_t size);
		~FreeListAllocator();

		[[nodiscard]] void* Allocate(size_t size);
		void Free(void* ptr);

	private:
		size_t* _data;
		size_t _size;
		size_t* _next;
	};

	inline FreeListAllocator::FreeListAllocator(size_t size) : _size(size)
	{
		size /= sizeof(size_t);

		_data = new size_t[size];
		_next = _data;

		*reinterpret_cast<void**>(_data) = nullptr;
		_next[1] = size - 2;
	}

	inline FreeListAllocator::~FreeListAllocator()
	{
		delete[] _data;
	}

	inline void* FreeListAllocator::Allocate(size_t size)
	{
		size = size / 4 + 2;

		size_t* current = _next;

		while(current)
		{
			auto& space = current[1];
			const auto next = reinterpret_cast<size_t*>(*current);

			if(space < size)
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

	inline void FreeListAllocator::Free(void* ptr)
	{
	}
}
