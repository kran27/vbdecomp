// Copyright 2024 OpenJE
#pragma once

#include <windows.h>
#include <shlobj.h>

namespace F3
{
	extern char g_szCurrentWorkingDirectory[260];
	extern char g_szSaveDirectoryAbsolute[260];
	extern char g_szSaveDirectoryRelative[260];

	void __cdecl SetupSaveDirectory(char* pszModulePath);
	char* __cdecl GetSaveDirectoryAbsolute();
	char* __cdecl GetSaveDirectoryRelative();
	char* __cdecl GetCurrentWorkingDirectory();
}
