#pragma once

namespace JE
{
	void __cdecl OpenLogFile(const char* pszFileName);
	void __cdecl CloseLogFile();

	// Library calls only; production uses the shared real MSVC filebuf.
	namespace LogFileServices
	{
		bool Open(const char* pszFileName);
		bool Close();
		bool IsOpen();
	}
}
