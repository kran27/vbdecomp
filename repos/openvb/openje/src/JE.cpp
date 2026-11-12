// OpenJE proprietary fatal reporting; MSVC supplies formatting and runtime services.
#include "diagnostics/DebugOutput.hpp"

#include <cstdio>
#include <cstdlib>

namespace JE
{
	// Original VA 0x48CC40. Caller formats a 1024-byte stack buffer, logs, dialogs, exits.
	void __cdecl FatalError(const char* pszFormat, ...)
	{
		char szBuffer[1024];
		va_list arguments;
		va_start(arguments, pszFormat);
		std::vsprintf(szBuffer, pszFormat, arguments);
		va_end(arguments);
		// Deliberately a second format pass: original passes only this buffer.
		LogDebugString(szBuffer);
		MessageBoxA(NULL, szBuffer, "Fatal Error", MB_SETFOREGROUND | MB_ICONERROR);
		std::exit(1);
	}

}
