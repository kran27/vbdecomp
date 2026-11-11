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
		DiagnosticsServices::Format(szBuffer, pszFormat, arguments);
		va_end(arguments);
		// Deliberately a second format pass: original passes only this buffer.
		LogDebugString(szBuffer);
		DiagnosticsServices::Message(NULL, szBuffer, "Fatal Error", MB_SETFOREGROUND | MB_ICONERROR);
		DiagnosticsServices::Terminate(1);
	}

	namespace DiagnosticsServices
	{
		int Format(char* pBuffer, const char* pszFormat, va_list arguments)
		{
			return std::vsprintf(pBuffer, pszFormat, arguments);
		}

		bool IsLogFileOpen()
		{
			return g_LogStream.rdbuf()->is_open();
		}

		void WriteLog(const char* pszText)
		{
			g_LogStream << pszText;
		}

		void FlushLog()
		{
			g_LogStream.flush();
		}

		void Debug(const char* pszText)
		{
			OutputDebugStringA(pszText);
		}

		int Message(HWND hWindow, const char* pszText, const char* pszCaption, UINT nType)
		{
			return MessageBoxA(hWindow, pszText, pszCaption, nType);
		}

		__declspec(noreturn) void Terminate(int nStatus)
		{
			std::exit(nStatus);
		}
	}
}
