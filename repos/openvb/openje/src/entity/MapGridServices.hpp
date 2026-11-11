#pragma once

#include <windows.h>
namespace JE { namespace Resources { struct ResourceFileHandle; } }
namespace JE { namespace MapGridServices
{
	// C++98 source boundary to the existing real native resource owner.
	Resources::ResourceFileHandle* Open(const char* name);
	BYTE Read(void* output, DWORD size, Resources::ResourceFileHandle* stream);
	void Release(Resources::ResourceFileHandle* stream);
}}
