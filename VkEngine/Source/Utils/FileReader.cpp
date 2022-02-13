#include "pch.h"
#include "Utils/FileReader.h"
#include <fstream>

vi::String FileReader::Read(const vi::String& path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	assert(file.is_open());

	// Dump contents in the buffer.
	const size_t fileSize = static_cast<size_t>(file.tellg());
	vi::String buffer{fileSize, GMEM_TEMP};
	
	file.seekg(0);
	file.read(buffer.GetData(), fileSize);
	file.close();
	return buffer;
}
