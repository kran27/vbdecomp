#include "ConfigurationSetup.hpp"
#include "configuration/Configuration.hpp"
#include "ConfigurationObject.hpp"
#include "SaveDirectory.hpp"
#include "Path.hpp"
#include "filesystem/FileOperations.hpp"
#include "JE.hpp"

#include <cstring>
#include <string>

namespace F3
{
	// 0x56B220.
	void __cdecl SetupConfigFile()
	{
		char szNewFileName[260];
		char szExistingFileName[260];
		std::strcpy(szNewFileName, GetSaveDirectoryAbsolute());
		char* const pszStem = GetSaveDirectoryRelative();
		PathConcat(szNewFileName, pszStem);
		std::strcat(szNewFileName, ".ini");
		if (!JE::FileExists(szNewFileName))
		{
			std::strcpy(szExistingFileName, GetCurrentWorkingDirectory());
			// Confirmed original destination: do not move this append to the
			// fallback source or remove the earlier .ini from the destination.
			PathConcat(szNewFileName, pszStem);
			std::strcat(szExistingFileName, ".ini");
			if (JE::FileExists(szExistingFileName))
			{
				if (!static_cast<unsigned char>(
					JE::CopyFileTo(szNewFileName, szExistingFileName, 0)))
				{
					JE::FatalError("Unable to create settings file %s from default file %s.",
						szNewFileName, szExistingFileName);
				}
			}
		}
		JE::Configuration::ReadFile(std::string(szNewFileName), *GetSettingsObject());
	}
}
