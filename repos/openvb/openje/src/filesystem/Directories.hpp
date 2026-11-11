#pragma once

namespace JE
{
	// 0x4C56B0. Enumerates the narrow path/pattern for any directory match.
	// The search handle is closed even when FindFirst returns -1.
	bool __cdecl DirectoryExists(const char* pszPath);

	// 0x4C5750. Input must fit the original 260-byte buffer and satisfy its
	// unchecked root/component delimiter scans.
	// The caller's string is never modified; failure keeps prior creations.
	bool __cdecl CreateDirectories(const char* pszPath);
}
