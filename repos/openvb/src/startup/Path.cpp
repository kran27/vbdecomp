#include "Path.hpp"

#include <cstring>

namespace F3
{
	// 0x4C58A0. See docs/functions/F3-004c58a0-PathConcat.md in the workspace.
	void __cdecl PathConcat(char* pszDestination, const char* pszSuffix)
	{
		unsigned int nLength = std::strlen(pszDestination);

		// 0x4C58B9 reads the preceding byte even when the destination is empty.
		char cLast = pszDestination[nLength - 1];
		if (cLast != '\\' && cLast != '/')
		{
			pszDestination[nLength] = '\\';
			pszDestination[nLength + 1] = '\0';
		}

		// Actual VC7 /O2 intrinsic: measure source, rescan destination, MOVSD/MOVSB.
		// This complete body matches 0x4C58A0; EAX is not a declared return value.
		std::strcat(pszDestination, pszSuffix);
	}
}
