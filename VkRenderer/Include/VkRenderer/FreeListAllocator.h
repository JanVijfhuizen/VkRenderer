#pragma once

namespace vi
{
	class FreeListAllocator final
	{
	public:
		explicit FreeListAllocator(size_t size);
		~FreeListAllocator();

		[[nodiscard]] void* Allocate(size_t size) const;
		void Free(void* ptr);

	private:
		void** _data;
		size_t _size;
		void** _next;
	};

	inline FreeListAllocator::FreeListAllocator(const size_t size) : _size(size)
	{
		_data = new void* [size];
		const auto s = reinterpret_cast<size_t*>(&_data[1]);
		*s = sizeof(void*) * (size - 1);
		_next = _data;
	}

	inline FreeListAllocator::~FreeListAllocator()
	{
		delete[] _data;
	}

	inline void* FreeListAllocator::Allocate(const size_t size) const
	{
		void** previous = nullptr;
		void** current = _next;

		while(true)
		{
			const auto cS = reinterpret_cast<size_t*>(&current[1]);
			if(*cS >= size)
			{
				void** next = &current[size];
				const auto nS = reinterpret_cast<size_t*>(&next[1]);
				*nS = *cS - size - sizeof(void*);

				if (previous)
					*previous = next;

				*current = next;
				*cS = size;
				return current;
			}

			previous = current;
			current = reinterpret_cast<void**>(*current);
		}
	}
}
