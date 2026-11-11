#include "FileOperations.hpp"
#include "LegacyFilesystem.hpp"

#include <windows.h>


namespace JE
{
	// 0x4C5590. Original decompiler void type is wrong: both exits explicitly
	// assign AL. Directories are skipped; close failure does not change the result.
	bool __cdecl FileExists(const char* pszPattern)
	{
		LegacyFilesystem::FindData32 data;
		const ::intptr_t handle = LegacyFilesystem::FindFirst(pszPattern, &data);
		if (handle != -1)
		{
			do
			{
				if (!(data.attrib & 0x10))
				{
					LegacyFilesystem::FindClose(handle);
					return true;
				}
			}
			while (LegacyFilesystem::FindNext(handle, &data) != -1);
		}
		LegacyFilesystem::FindClose(handle);
		return false;
	}

	// 0x4C5630. Do not swap the public argument order to mimic the Win32 API.
	int __cdecl CopyFileTo(const char* pszDestination, const char* pszSource,
		unsigned char failIfExists)
	{
		const BOOL result = ::CopyFileA(pszSource, pszDestination, failIfExists);
		return result != 0 ? 1 : 0;
	}
}
