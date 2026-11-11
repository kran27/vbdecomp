// Proprietary INI serialization, original VA 0x4DF1C0; actual MSVC library objects.
#include "Configuration.hpp"

namespace JE
{
	namespace Configuration
	{
		unsigned char __cdecl WriteFile(std::string sFilename, const Settings& settings)
		{
			std::string sLine;
			FILE* pFile = WriteFileServices::Open(sFilename.c_str(), "wt");
			if (!pFile)
			{
				return 0;
			}
			Settings::const_iterator section;
			for (section = settings.begin(); section != settings.end();
				section++)
			{
				sLine = "[" + section->first + "]\n";
				if (WriteFileServices::PutString(sLine.c_str(), pFile) == EOF)
				{
					WriteFileServices::Close(pFile);
					return 0;
				}
				const Values& values = section->second;
				Values::const_iterator value;
				for (value = values.begin(); value != values.end(); value++)
				{
					sLine = value->first + " = " + value->second + "\n";
					if (WriteFileServices::PutString(sLine.c_str(), pFile) == EOF)
					{
						WriteFileServices::Close(pFile);
						return 0;
					}
				}
				if (WriteFileServices::PutString("\n", pFile) == EOF)
				{
					WriteFileServices::Close(pFile);
					return 0;
				}
			}
			// Original success ignores a failed final flush/close.
			WriteFileServices::Close(pFile);
			return 1;
		}

		namespace WriteFileServices
		{
			FILE* __cdecl Open(const char* pszFilename, const char* pszMode)
			{
				return std::fopen(pszFilename, pszMode);
			}

			int __cdecl PutString(const char* pszText, FILE* pFile)
			{
				return std::fputs(pszText, pFile);
			}

			int __cdecl Close(FILE* pFile)
			{
				return std::fclose(pFile);
			}
		}
	}
}
