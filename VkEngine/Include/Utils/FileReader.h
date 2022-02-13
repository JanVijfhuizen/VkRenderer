#pragma once

// Utility class that reads data from a file.
class FileReader final
{
public:
	// Reads from a file and returns a string with the contents.
	static vi::String Read(const vi::String& path);
};
