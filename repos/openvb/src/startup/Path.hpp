#pragma once

namespace F3
{
	// 0x4C58A0. Appends to a writable, NUL-terminated destination with spare capacity.
	// An empty destination reads destination[-1], just as the original routine does.
	// Aliases preserve the original forward four-byte copies followed by byte copies.
	// All ten original callers discard EAX; VC7's strcat leaves an incidental count.
	void __cdecl PathConcat(char* pszDestination, const char* pszSuffix);
}
