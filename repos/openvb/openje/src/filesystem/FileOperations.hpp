#pragma once

namespace JE
{
	// Original 0x4C5590 searches a pattern for any non-directory match.
	// The failed handle -1 is still passed to the original close boundary.
	bool __cdecl FileExists(const char* pszPattern);

	// Original 0x4C5630 takes destination FIRST, source SECOND. Forward the
	// complete low flag byte to CopyFileA, then normalize BOOL to full int 0/1.
	int __cdecl CopyFileTo(const char* pszDestination, const char* pszSource,
		unsigned char failIfExists);
}
