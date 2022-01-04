#pragma once

namespace vi
{
	/// <summary>
	/// Minimal string class that also functions as an array.
	/// </summary>
	class String final : public ArrayPtr<char>
	{
	public:
		String();
		explicit String(FreeListAllocator& allocator);
		String(const char* buffer, FreeListAllocator& allocator);
		[[nodiscard]] operator const char* () const;

		/// <summary>
		/// Add the contents of another string/array of chars to this string.
		/// </summary>
		void Append(const char* buffer);
	};
}
