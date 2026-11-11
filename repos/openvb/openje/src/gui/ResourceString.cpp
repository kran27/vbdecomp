#include "ResourceIO.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstring>
#include <new>

namespace GUI
{
	BYTE ReadResourceString(std::string& value, JE::Resources::ResourceFileHandle* file)
	{
		DWORD size = static_cast<DWORD>(value.size());
		if (!JE::Resources::g_pResourceSystem->Read(&size, 4, file)) return 0;
		char* bytes = new char[static_cast<DWORD>(size + 1)];
		if (!JE::Resources::g_pResourceSystem->Read(bytes, size, file)) return 0; // Original buffer leak.
		bytes[size] = 0;
		value.assign(bytes, std::strlen(bytes));
		delete[] bytes;
		return 1;
	}
}
