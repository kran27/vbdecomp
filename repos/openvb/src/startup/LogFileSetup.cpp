#include "LogFileSetup.hpp"
#include "SaveDirectory.hpp"
#include "Path.hpp"
#include "diagnostics/LogFile.hpp"

#include <cstring>

namespace F3
{
	// Original VA 0x56B100. Valid domain: final pathname fits 260 bytes.
	void SetupLogFile()
	{
		char szFilePath[260];
		std::strcpy(szFilePath, GetSaveDirectoryAbsolute());
		PathConcat(szFilePath, GetSaveDirectoryRelative());
		std::strcat(szFilePath, ".log");
		JE::OpenLogFile(szFilePath);
	}

	// Original VA 0x56B190 is a tail jump to the engine's close wrapper.
	void ShutdownLogFile()
	{
		JE::CloseLogFile();
	}
}
