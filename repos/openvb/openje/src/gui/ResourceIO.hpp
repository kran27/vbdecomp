#pragma once

#include <windows.h>
#include <string>

namespace JE { namespace Resources { struct ResourceFileHandle; } }

namespace GUI
{
	// Original utility50B880; length-prefixed, C-string assignment semantics.
	BYTE ReadResourceString(std::string& value, JE::Resources::ResourceFileHandle* file);

	// Explicit C++98 source boundary to the existing native ResourceSystem.
	// Every operation reloads the actual global; no original storage is overlaid.
	namespace ResourceIO
	{
		BYTE Read(void* output, DWORD size, JE::Resources::ResourceFileHandle* file);
		JE::Resources::ResourceFileHandle* Open(const char* name);
		BYTE Seek(JE::Resources::ResourceFileHandle* file, LONG offset, int origin);
		void Release(JE::Resources::ResourceFileHandle* file);
	}
}
