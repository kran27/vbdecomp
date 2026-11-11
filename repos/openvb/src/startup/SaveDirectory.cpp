// Copyright 2024 OpenJE
#include "SaveDirectory.hpp"
#include "Path.hpp"
#include "JE.hpp"
#include <direct.h>
#include <stdlib.h>
#include "filesystem/Directories.hpp"

namespace F3
{
	// Original globals: cwd 0x70BFA8, absolute save path 0x70C0B0, stem 0x70C1B8.
	char g_szCurrentWorkingDirectory[260] = { 0 };
	char g_szSaveDirectoryAbsolute[260] = { 0 };
	char g_szSaveDirectoryRelative[260] = { 0 };

	// 0x56AF00. No ownership transfer; callers share this mutable global.
	char* __cdecl GetSaveDirectoryAbsolute()
	{
		return g_szSaveDirectoryAbsolute;
	}

	// 0x56AF10. "Relative" is the executable filename stem from SplitPath.
	char* __cdecl GetSaveDirectoryRelative()
	{
		return g_szSaveDirectoryRelative;
	}

	// 0x56AF20. Shares the original cwd capture with config/resource callers.
	char* __cdecl GetCurrentWorkingDirectory()
	{
		return g_szCurrentWorkingDirectory;
	}

	// 0x56B000. Evidence: docs/functions/F3-0056b000-SetupSaveDirectory.md.
	void __cdecl SetupSaveDirectory(char* pszModulePath)
	{
		char szPersonalPath[260];
		if (!::_getcwd(g_szCurrentWorkingDirectory, 260))
		{
			JE::FatalError("Unable to retrieve current working directory.");
		}
		if (SHGetFolderPathA(NULL, CSIDL_FLAG_CREATE | CSIDL_PERSONAL, NULL, 0,
			szPersonalPath) != 0)
		{
			JE::FatalError("Unable to retrieve personal folder.");
		}

		int nIndex = 0;
		do
		{
			g_szSaveDirectoryAbsolute[nIndex] = szPersonalPath[nIndex];
		}
		while (szPersonalPath[nIndex++]);

		::_splitpath(pszModulePath, NULL, NULL, szPersonalPath, NULL);
		nIndex = 0;
		do
		{
			g_szSaveDirectoryRelative[nIndex] = szPersonalPath[nIndex];
		}
		while (szPersonalPath[nIndex++]);

		PathConcat(g_szSaveDirectoryAbsolute, GetSaveDirectoryRelative());
		if (!JE::DirectoryExists(g_szSaveDirectoryAbsolute))
		{
			if (!JE::CreateDirectories(g_szSaveDirectoryAbsolute))
			{
				JE::FatalError("Unable to create save folder.");
			}
		}
	}
}
