#include "ConfigurationSave.hpp"
#include "configuration/Configuration.hpp"
#include "ConfigurationObject.hpp"
#include "SaveDirectory.hpp"
#include "Path.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <cstring>

namespace F3
{
	// Original VA 0x56B390. The final pathname must fit the original 260-byte array.
	void SaveSettingsFile()
	{
		char szFilePath[260];
		std::strcpy(szFilePath, GetSaveDirectoryAbsolute());
		PathConcat(szFilePath, GetSaveDirectoryRelative());
		std::strcat(szFilePath, ".ini");
		if (!JE::Configuration::WriteFile(std::string(szFilePath), g_SettingsObject))
		{
			JE::FatalError("Unable to save settings file %s.", szFilePath);
		}
	}
}
