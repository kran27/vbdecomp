#pragma once

#include "Configuration.hpp"

namespace JE
{
	namespace Configuration
	{
		// 0x4E0960. Uses the canonical resource manager, reloaded at each call.
		// This is distinct from ReadFile's direct filesystem route.
		unsigned char __cdecl ReadResourceConfigurationFile(std::string sFilename,
			Settings& settings);

		namespace ResourceConfigurationServices
		{
			// The default reuses FileServices' documented extra physical byte for
			// the parser. The game-owned logical request remains DWORD length+1.
			void* __cdecl Allocate(Net::UInt32 nBytes);
			void __cdecl Free(void* pBuffer);
		}
	}
}
