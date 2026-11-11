#pragma once

#include "configuration/ConfigurationTypes.hpp"

namespace F3
{
	// Distinct original settings identity 0x70C2C0, not main settings 0x70C2CC.
	extern JE::Configuration::Settings g_EngineSettings;
	JE::Configuration::Settings* __cdecl GetEngineSettings(); // 0x56AF40
}
