#pragma once

#include "configuration/ConfigurationTypes.hpp"
#include <windows.h>

namespace JE { namespace ModelServices
{
	// C++98 API boundary to the reconstructed reader/resource implementation.
	BYTE ReadSettings(const std::string& name, Configuration::Settings& settings);
	BYTE ResourceExists(const char* name); // 0x49ABD0; locked temp before primary.
	std::string UpperName(const std::string& name); // Real CRT _strupr, original260-byte input domain.
	BYTE LoadSocketTexture(DWORD* output, const std::string& name, HRESULT* result);
}}
