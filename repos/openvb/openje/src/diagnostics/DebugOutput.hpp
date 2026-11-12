#pragma once

#include <cstdarg>
#include <fstream>
#include <ostream>
#include <windows.h>

namespace JE
{
	// One shared byte, corresponding to original 0x707CF0. Not thread local.
	extern BYTE g_bLogging;
	// One real fstream owner; its file buffer and stream bases are library subobjects.
	extern std::fstream g_LogStream;

	void __cdecl LogDebugString(const char* pszFormat, ...);
	void __cdecl FatalError(const char* pszFormat, ...);


}
