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

	// Thin dependency interfaces, implemented by real libraries by default.
	// A single test macro omits those defaults and supplies controlled services.
	namespace DiagnosticsServices
	{
		int Format(char* pBuffer, const char* pszFormat, va_list arguments);
		bool IsLogFileOpen();
		void WriteLog(const char* pszText);
		void FlushLog();
		void Debug(const char* pszText);
		int Message(HWND hWindow, const char* pszText, const char* pszCaption, UINT nType);
		__declspec(noreturn) void Terminate(int nStatus);
	}
}
