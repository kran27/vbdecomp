#pragma once

#include <windows.h>

namespace JE { namespace Common
{
	BYTE ExtractExtension(char* destination, const char* source); // 0x4C5650; last dot, no path normalization.
}}
