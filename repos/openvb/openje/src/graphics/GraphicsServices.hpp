#pragma once

#include "configuration/ConfigurationTypes.hpp"
#include <windows.h>
#include <string>

namespace JE { namespace GraphicsServices
{
	//Actual startup providers56AF30/56AF40/70C0AC/56C280. The game startup
	//module supplies these bridges; there is no second settings or window owner.
	Configuration::Settings& MainSettings();
	Configuration::Settings& EngineSettings();
	HWND Window();
	BYTE __cdecl ResolveString(DWORD reference, std::string& output);
}}
