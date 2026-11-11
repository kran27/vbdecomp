#include "FileString.hpp"
#include <cstdlib>
#include <cstring>
#include <new>

namespace GUI
{
	BYTE ReadFileString(std::string& value, FILE* file)
	{
		DWORD length = static_cast<DWORD>(value.size());
		if (std::fread(&length, 4, 1, file) != 1) return 0;
		char* buffer = new char[static_cast<DWORD>(length + 1)];
		if (std::fread(buffer, 1, length, file) != length) return 0; //Original leak.
		buffer[length] = 0;
		value.assign(buffer, std::strlen(buffer)); //Original embedded-NUL truncation.
		delete[] buffer;
		return 1;
	}

	BYTE WriteFileString(const std::string& value, FILE* file)
	{
		DWORD length = static_cast<DWORD>(value.size());
		if (std::fwrite(&length, 4, 1, file) != 1) return 0;
		return std::fwrite(value.c_str(), 1, length, file) == length;
	}
}
