// Proprietary SendDebugString, original VA 0x497120; real compiler library storage.
#include "DebugOutput.hpp"
#include <cstdio>

namespace JE
{
	BYTE g_bLogging = 0;
	std::fstream g_LogStream;

	void __cdecl LogDebugString(const char* pszFormat, ...)
	{
		if (g_bLogging)
		{
			return;
		}

		g_bLogging = 1;
		char szBuffer[1024];
		va_list arguments;
		va_start(arguments, pszFormat);
		std::vsprintf(szBuffer, pszFormat, arguments);
		va_end(arguments);

		// The original checks the filebuf's FILE* slot, not a readiness flag.
		if (g_LogStream.rdbuf()->is_open())
		{
			g_LogStream << szBuffer;
			g_LogStream.flush();
		}
		OutputDebugStringA(szBuffer);
		// No exception rollback: original has no local cleanup for this byte.
		g_bLogging = 0;
	}
}
