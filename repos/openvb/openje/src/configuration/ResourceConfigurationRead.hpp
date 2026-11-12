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

	}
}
