#pragma once

#include <windows.h>

namespace JE { namespace Resources
{
	struct ResourceData;
	struct ResourceMemoryStream
	{
		ResourceData* resource;
		const BYTE* begin;
		const BYTE* end;
		const BYTE* cursor;
	};

	// Typed bridge to the actual resource owner. The C++98 graphics units need
	// no knowledge of its STL representation or original ResourceData offsets.
	ResourceMemoryStream* OpenResourceMemoryStream(const char* name, BYTE cached);
	void CloseResourceMemoryStream(ResourceMemoryStream* stream);
}}
