// Copyright 2024 OpenJE
#pragma once

#include "configuration/ConfigurationTypes.hpp"

namespace F3
{
	// Source-level owner of the original canonical settings identity 0x70C2CC.
	// The actual MSVC std::map supplies construction, destruction and storage.
	extern JE::Configuration::Settings g_SettingsObject;
	JE::Configuration::Settings* __cdecl GetSettingsObject(); // 0x56AF30
}
