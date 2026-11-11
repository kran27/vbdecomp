#include "LogFile.hpp"
#include "DebugOutput.hpp"

namespace JE
{
	// Original VA 0x498000. Stream state belongs to the actual C++ library.
	void __cdecl OpenLogFile(const char* pszFileName)
	{
		char szUnwrittenArgument[260];
		if (!LogFileServices::Open(pszFileName))
		{
			g_LogStream.setstate(std::ios::failbit);
		}
		if (g_LogStream.fail())
		{
			// Original 0x49806A passes this unwritten local, NOT pszFileName.
			// Error text depending on its bytes is unresolved and not validated.
			FatalError("Unable to open log file %s.", szUnwrittenArgument);
		}
	}

	// Original VA 0x497B70. Caller ignores its incidental library return value.
	void __cdecl CloseLogFile()
	{
		if (LogFileServices::IsOpen() && !LogFileServices::Close())
		{
			g_LogStream.setstate(std::ios::failbit);
		}
	}

	namespace LogFileServices
	{
		bool Open(const char* pszFileName)
		{
			// Original mode 0x12 selects "w": text output, create/truncate.
			return g_LogStream.rdbuf()->open(pszFileName, std::ios::out | std::ios::trunc) != NULL;
		}

		bool Close()
		{
			return g_LogStream.rdbuf()->close() != NULL;
		}

		bool IsOpen()
		{
			return g_LogStream.rdbuf()->is_open();
		}
	}
}
