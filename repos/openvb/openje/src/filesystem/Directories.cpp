#include "Directories.hpp"
#include "LegacyFilesystem.hpp"

#include <cstddef>
#include <cstring>

namespace JE
{
	typedef char FindDataWidthRequired[(sizeof(LegacyFilesystem::FindData32) == 280) ? 1 : -1];
	typedef char FindDataAttributesRequired[
		(offsetof(LegacyFilesystem::FindData32, attrib) == 0) ? 1 : -1];

	// 0x4C56B0. See docs/functions/F3-004c56b0-DirectoryExists.md in the workspace.
	bool __cdecl DirectoryExists(const char* pszPath)
	{
		LegacyFilesystem::FindData32 data;
		const ::intptr_t hFind = LegacyFilesystem::FindFirst(pszPath, &data);
		if (hFind != -1)
		{
			do
			{
				if (data.attrib & 0x10)
				{
					LegacyFilesystem::FindClose(hFind);
					return true;
				}
			}
			while (LegacyFilesystem::FindNext(hFind, &data) != -1);
		}

		// Both original exits close the handle; the false exit also closes -1.
		LegacyFilesystem::FindClose(hFind);
		return false;
	}

	// 0x4C5750. See docs/functions/F3-004c5750-CreateDirectories.md in the workspace.
	bool __cdecl CreateDirectories(const char* pszPath)
	{
		char szPath[260];
		std::strcpy(szPath, pszPath);
		char* const pszLast = szPath + std::strlen(szPath) - 1;
		if (*pszLast == '\\' || *pszLast == '/')
		{
			*pszLast = '\0';
		}

		char* pszCursor = szPath;
		if (*pszCursor == '\\' || *pszCursor == '/')
		{
			if (szPath[1] == '\\' || szPath[1] == '/')
			{
				pszCursor = szPath + 2;
				int nComponents = 2;
				do
				{
					// UNC server/share scans intentionally have no NUL check.
					while (*pszCursor != '\\' && *pszCursor != '/')
					{
						++pszCursor;
					}
					++pszCursor;
					--nComponents;
				}
				while (nComponents);
			}
		}
		else
		{
			// The original skips the first component; this scan has no NUL check.
			while (*pszCursor != '/')
			{
				if (*++pszCursor == '\\')
				{
					++pszCursor;
					break;
				}
			}
		}

		while (*pszCursor == '\\' || *pszCursor == '/')
		{
			++pszCursor;
		}
		if (*pszCursor)
		{
			do
			{
				if (*pszCursor == '\\' || *pszCursor == '/')
				{
					*pszCursor = '\0';
					if (!DirectoryExists(szPath) && LegacyFilesystem::MakeDirectory(szPath) == -1)
					{
						return false;
					}
					*pszCursor = '\\';
				}
			}
			while (*++pszCursor);
		}

		if (!DirectoryExists(szPath) && LegacyFilesystem::MakeDirectory(szPath) == -1)
		{
			return false;
		}
		return true;
	}
}
