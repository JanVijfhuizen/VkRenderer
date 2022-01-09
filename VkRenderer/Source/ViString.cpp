#include "pch.h"
#include "ViString.h"

namespace vi
{
	String::String() = default;

	String::String(FreeListAllocator& allocator) : ArrayPtr<char>(1, allocator, '\0')
	{

	}

	String::String(const char* buffer, FreeListAllocator& allocator)
	{
		const size_t size = strlen(buffer);
		Reallocate(size + 1, allocator);
		memcpy(GetData(), buffer, sizeof(char) * size);
		operator[](GetLength() - 1) = '\0';
	}

	String::String(const size_t size, FreeListAllocator& allocator)
	{
		Reallocate(size, allocator);
	}

	String::operator const char*() const
	{
		assert(GetLength() > 0);
		return GetData();
	}

	void String::Append(const char* buffer)
	{
		const size_t size = strlen(buffer);
		const size_t length = GetLength();
		Reallocate(length + size, *GetAllocator());

		const auto data = GetData();
		memcpy(&data[length - 1], buffer, sizeof(char) * size);
		data[GetLength() - 1] = '\0';
	}
}
